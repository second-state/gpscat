#include <llvm/Support/CommandLine.h>

#include <symengine/expression.h>
#include <symengine/symbol.h>
#include <symengine/basic.h>
#include <symengine/dict.h>
#include <symengine/real_double.h>

#include <boost/math/quadrature/naive_monte_carlo.hpp> 
#include <boost/math/quadrature/gauss_kronrod.hpp> 

#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <map>
#include <future>
#include <chrono>
#include <thread>
#include <limits>

static llvm::cl::opt<std::string> cmdInputFunction(llvm::cl::Positional, llvm::cl::desc("<function>"), llvm::cl::init("-"));
static llvm::cl::opt<std::string> boundsFilename("bounds-file", llvm::cl::desc("File for specifying bounds"), llvm::cl::init(""));
// Recommended precision : float : 9, double : 17
static llvm::cl::opt<int> printPrecision("print-precision", llvm::cl::desc("Precision of the output"), llvm::cl::init(17));
static llvm::cl::opt<unsigned int> verbosity("verbose", llvm::cl::desc("Verbosity"), llvm::cl::init(0));
static llvm::cl::opt<unsigned int> maxtime("maxtime", llvm::cl::desc("Maximum waiting time (in seconds) when using Monte-Carlo integration"), llvm::cl::init(3));
static llvm::cl::opt<unsigned int> numThreads("jobs", llvm::cl::desc("Number of parallel threads when using Monte-Carlo integration"),
                                                  llvm::cl::init(std::max(0u, std::thread::hardware_concurrency() - 1)));
static llvm::cl::opt<std::string> numericalIntegrationAlgo("algorithm", llvm::cl::desc("Specify the algorithm used for numerical integration"), llvm::cl::init("auto"));

std::set<std::string> getSymbols(SymEngine::RCP<const SymEngine::Basic> x) {
    if(SymEngine::is_a<SymEngine::Symbol>(*x))
        return {x->__str__()};

    std::set<std::string> S;
    for(const auto &arg : x->get_args()) {
        auto &&argSymbols = getSymbols(arg);
        S.insert(std::begin(argSymbols), std::end(argSymbols));
    }
    return S;
}

double MonteCarloIntegration(SymEngine::Expression func, const SymEngine::vec_sym &params, const std::vector<std::pair<double, double>> &bounds) {
    /* Monte Carlo is suitable for numerically integrate functions with
     * many variables. However, it has slow convergence.
     */
    using boost::math::quadrature::naive_monte_carlo;

    auto integrand = [&params, func](const std::vector<double> &x) -> double {
        SymEngine::map_basic_basic subsMap;
        for(std::size_t i = 0; i < params.size(); ++i)
            subsMap[params[i]] = SymEngine::real_double(x[i]);
        return static_cast<double>(func.subs(subsMap));
    };

    naive_monte_carlo<double, decltype(integrand)> mc(integrand, bounds, 1e-3, true, numThreads);

    std::future<double> task = mc.integrate();
    unsigned secondCount = 0;
    while(task.wait_for(std::chrono::seconds(1)) != std::future_status::ready) {
        // display plain progress information
        if(verbosity >= 1) {
            std::cout << mc.progress() << '\t'
                      << mc.current_error_estimate() << '\t'
                      << mc.current_estimate() << '\t'
                      << mc.estimated_time_to_completion().count() << '\t'
                      << mc.calls() << std::endl;
        }
        ++secondCount;
        if(secondCount >= maxtime) {
            mc.cancel();
            if(verbosity >= 1)
                std::cout << "Stop calculating because it took too much time" << std::endl;
        }
    }

    double result = task.get();
    return result;
}

double GaussKronrodIntegration(SymEngine::Expression func, const SymEngine::vec_sym &params, const std::vector<std::pair<double, double>> &bounds, const std::size_t currentIndex = 0) {
    /* Gauss Kronrod becomes extremely slow when number of params
     * is large. Also, it assumes the conditions of Fubini's theorem
     * is satisfied when doing multidimensional integration
     */
    using boost::math::quadrature::gauss_kronrod;

    // iteratively evaluate the integrals
    if(currentIndex + 1 == params.size()) {
        auto integrand = [&params, currentIndex, func](double t) -> double {
            return static_cast<double>(func.subs({{params[currentIndex], SymEngine::real_double(t)}}));
        };
        double result = gauss_kronrod<double, 15>::integrate(integrand, bounds[currentIndex].first, bounds[currentIndex].second);
        return result;
    }
    else {
        auto integrand = [&params, &bounds, currentIndex, func](double t) -> double {
            return GaussKronrodIntegration(func.subs({{params[currentIndex], SymEngine::real_double(t)}}), params, bounds, currentIndex + 1);
        };
        double result = gauss_kronrod<double, 15>::integrate(integrand, bounds[currentIndex].first, bounds[currentIndex].second);
        return result;
    }
}

double numericallyIntegrate(SymEngine::Expression func, const std::vector<std::string> &paramsName, const std::map<std::string, std::pair<int, int>> &boundsMap) {
    SymEngine::vec_sym params;
    for(const auto &paramName : paramsName)
        params.push_back(SymEngine::symbol(paramName));

    std::vector<std::pair<double, double>> bounds;
    for(const auto &singleVarBounds : boundsMap)
        bounds.push_back(singleVarBounds.second);

    // Special case where the function is a constant
    if(params.size() == 0) {
        return static_cast<double>(func);
    }

    if(numericalIntegrationAlgo == "auto") {
        if(paramsName.size() <= 4)
            return GaussKronrodIntegration(func, params, bounds);
        else
            return MonteCarloIntegration(func, params, bounds);
    }
    else if(numericalIntegrationAlgo == "monte_carlo") {
        return MonteCarloIntegration(func, params, bounds);
    }
    else if(numericalIntegrationAlgo == "gauss_kronrod") {
        return GaussKronrodIntegration(func, params, bounds);
    }
    else {
        std::cerr << "The numerical integration algorithm is not supported." << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
    }
}

int main(int argc, char *argv[]) {
    llvm::cl::ParseCommandLineOptions(argc, argv);

    // Set output format
    std::cout << std::fixed;
    std::cout.precision(printPrecision);

    // Read function
    std::string inputFunction;
    if(cmdInputFunction != "-")
        inputFunction = cmdInputFunction;
    else
        std::getline(std::cin, inputFunction);

    SymEngine::Expression func(inputFunction);

    // Handle infinity
    if(func == SymEngine::Expression("oo") || func == SymEngine::Expression("-oo")) {
        std::cout << func << std::endl;
        return 0;
    }

    // Get function parameters
    std::set<std::string> paramsSet = getSymbols(func);
    if(verbosity >= 1) {
        std::cout << "Parameter set: ";
        for(const auto &param : paramsSet)
            std::cout << param << ' ';
        std::cout << std::endl;
    }

    // Read argument bounds
    std::map<std::string, std::pair<int, int>> bounds;
    std::string variableName;
    int lowerBound, upperBound;

    if(!boundsFilename.empty()) {
        std::ifstream boundsFile(boundsFilename);
        while(boundsFile >> variableName >> lowerBound >> upperBound) {
            if(lowerBound > upperBound)
                std::cerr << "Lowerbound is greater than upperbound!" << std::endl;
            else if(paramsSet.find(variableName) != paramsSet.end())
                bounds[variableName] = {lowerBound, upperBound};
        }
    }
    else {
        // read bounds from stdin until EOF or "end"
        while(std::cin >> variableName && variableName != "end") {
            std::cin >> lowerBound >> upperBound;
            if(lowerBound > upperBound)
                std::cerr << "Lowerbound is greater than upperbound!" << std::endl;
            else if(paramsSet.find(variableName) != paramsSet.end())
                bounds[variableName] = {lowerBound, upperBound};
        }
    }

    if(bounds.size() != paramsSet.size()) {
        std::cerr << "Some variables are unbounded" << std::endl;
        return 1;
    }

    auto result = numericallyIntegrate(func, std::vector<std::string>(paramsSet.begin(), paramsSet.end()), bounds);
    
    // Calculate the mean of the input function
    for(const auto& singleVarBounds : bounds) {
        int width = singleVarBounds.second.second - singleVarBounds.second.first;
        if(width == 0) continue;
        result /= width;
    }

    std::cout << result << std::endl;

    return 0;
}
