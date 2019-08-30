#include <gpscat/AssemblyCostModel.h>
#include <gpscat/IRLocator.h>
#include <gpscat/MappingExtractor.h>
#include <gpscat/IRCostCalculator.h>
#include <gpscat/CoFloCoWrapper.h>
#include <gpscat/Utils.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/SourceMgr.h>

#include <symengine/expression.h>

#include <iostream>
#include <string>
#include <system_error>
#include <cassert>

static llvm::cl::opt<std::string> costModelFilename(llvm::cl::Positional, llvm::cl::desc("<cost model csv file>"), llvm::cl::Required);
static llvm::cl::opt<std::string> inputFilename(llvm::cl::Positional, llvm::cl::desc("<input bitcode file>"), llvm::cl::init("-"));
static llvm::cl::opt<std::string> arch("arch", llvm::cl::desc("Target assembly language"), llvm::cl::init("wasm32"));
static llvm::cl::opt<bool> keepTemporaryFiles("keep-temporary-files", llvm::cl::desc("Don't remove the temporary files when analyzing cost"));
static llvm::cl::opt<int> verbosity("verbose", llvm::cl::desc("verbosity level (0, 1, 2)"), llvm::cl::init(0));
static llvm::cl::opt<std::string> optLevel("O", llvm::cl::Prefix, llvm::cl::desc("Optimization level for llc"), llvm::cl::init("2"));
static llvm::cl::opt<bool> removeNat("remove-nat", llvm::cl::desc("Remove all occurrences of nat(x) (Can lead to incorrect upperbounds)"));
static llvm::cl::opt<bool> replaceNat("replace-nat", llvm::cl::desc("Replace all nat(x) with max([x,0])"));
static llvm::cl::opt<bool> symengineFormat("symengine-format", llvm::cl::desc("Print the upperbound in SynEngine format"));

using namespace std::literals;

int main(int argc, char *argv[]) {
    llvm::cl::ParseCommandLineOptions(argc, argv);

    // Read bitcode file
    if(verbosity >= 1) std::cout << "Reading input bitcode file." << std::endl;

    llvm::LLVMContext context;
    llvm::SMDiagnostic err;
    std::unique_ptr<llvm::Module> module = parseIRFile(inputFilename, err, context);

    if(!module) {
        err.print(argv[0], llvm::errs());
        return 1;
    }

    // Read cost model
    if(verbosity >= 1) std::cout << "Reading cost model." << std::endl;

    gpscat::AssemblyCostModel costModel(costModelFilename);
    
    // Use IRLocator to create LLVM IR to ASM mapping information
    if(verbosity >= 1) std::cout << "Creating LLVM IR to ASM mapping information." << std::endl;

    gpscat::IRLocator irLocator;
    irLocator.run(module.get());

    // Compile this module into target language
    if(verbosity >= 1) std::cout << "Compiling into target language." << std::endl;

    std::string mappingBitcodePath = gpscat::getTemporaryFilePath("gpscat-cost", "map.bc");
    std::string mappingAsmPath = gpscat::getTemporaryFilePath("gpscat-cost", "map.s");
    gpscat::writeBitcodeFile(module.get(), mappingBitcodePath);

    llvm::ErrorOr<std::string> llcPath = llvm::sys::findProgramByName("llc");
    if(std::error_code ec = llcPath.getError()) {
        std::cerr << ec << std::endl
                  << ec.message() << std::endl;
        return 3;
    }
    llvm::sys::ExecuteAndWait(llcPath.get(),
                              {"llc"s, mappingBitcodePath, "-o"s, mappingAsmPath, "-march="s + arch, "-O"s + optLevel});

    // Extract LLVM IR to ASM mapping
    if(verbosity >= 1) std::cout << "Extract mapping information." << std::endl;

    gpscat::MappingExtractor mappingExtractor;
    const auto& IRAsmMap = mappingExtractor.extractMapping(mappingAsmPath, costModel, module.get());

    // Print mapping
    if(verbosity >= 2) {
        std::cout << "\n--------LLVM IR to Assembly mapping--------\n";
        // print LLVM IR instructions by their original order
        uint32_t lineNumber = 0;
        for(auto &&F : *module) {
            for(auto &&B : F) {
                for(auto &&I : B) {
                    lineNumber++;
                    std::cout << lineNumber << '\t';

                    auto &&instOpcodeName = std::string(I.getOpcodeName());
                    instOpcodeName.resize(20, ' ');
                    std::cout << instOpcodeName << '\t';

                    auto it = IRAsmMap.find(&I);
                    if(it != IRAsmMap.end()) {
                        for(const auto& asmInst : it->second)
                            std::cout << asmInst.getName() << ' ';
                    }
                    std::cout << '\n';
                }
            }
        }
        std::cout.flush();
    }

    // Calculate LLVM IR block level costs
    if(verbosity >= 1) std::cout << "Calculating LLVM IR block level costs." << std::endl;

    gpscat::IRCostCalculator irCostCalculator(module.get(), costModel, IRAsmMap);

    if(verbosity >= 2) {
        // Print inst cost
        {
        std::cout << "\n--------Instruction cost--------\n";
        uint32_t lineNumber = 0;
        for(auto &&F : *module) {
            for(auto &&B : F) {
                for(auto &&I : B) {
                    lineNumber++;

                    auto &&instOpcodeName = std::string(I.getOpcodeName());
                    instOpcodeName.resize(20, ' ');
                    std::cout << lineNumber << '\t' << instOpcodeName << '\t'
                              << irCostCalculator.getInstCost(&I) << '\n';
                }
            }
        }
        std::cout.flush();
        }
    
        // Print block cost
        {
        std::cout << "\n--------Block cost--------\n";
        uint32_t lineNumber = 0;
        for(auto &&F : *module) {
            for(auto &&B : F) {
                lineNumber++;
                std::cout << lineNumber << '\t'
                          << irCostCalculator.getBlockCost(&B) << '\n';
            }
        }
        std::cout.flush();
        }
    }

    // Call upperbound solver to calculate LLVM IR function level cost
    if(verbosity >= 1) std::cout << "Calculating LLVM IR function level cost." << std::endl;

    gpscat::CoFloCoWrapper coflocoWrapper;

    if(verbosity >= 1) std::cout << "\tAnnotating cost information." << std::endl;
    coflocoWrapper.cost2tick(module.get(), irCostCalculator.getBlockCostMap());

    if(verbosity >= 1) std::cout << "\tExtracting cost relation system." << std::endl;
    std::string crsPath = gpscat::getTemporaryFilePath("gpscat-cost", "tmp.ces");
    coflocoWrapper.extractCostRelationSystem(module.get(), crsPath);

    if(verbosity >= 1) std::cout << "\tSolving cost upperbound." << std::endl;
    std::string costUpperBound = coflocoWrapper.readCRSAndSolveUpperBound(crsPath);

    if(removeNat) costUpperBound = coflocoWrapper.removeNat(costUpperBound);
    if(replaceNat) costUpperBound = coflocoWrapper.replaceNatWithMax(costUpperBound);

    if(symengineFormat) {
        SymEngine::Expression symUpperBound(costUpperBound);
        costUpperBound = expand(symUpperBound).get_basic()->__str__();
    }

    // Output cost upperbound
    if(verbosity >= 1) std::cout << "\nThe inferred cost upperbound is" << std::endl;
    std::cout << costUpperBound << std::endl;

    // Remove temporary file
    if(!keepTemporaryFiles) {
        llvm::sys::fs::remove(mappingBitcodePath);
        llvm::sys::fs::remove(mappingAsmPath);
        llvm::sys::fs::remove(crsPath);
    }

    return 0;
}
