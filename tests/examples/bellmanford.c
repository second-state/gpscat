int bellmanford(int n, int m, int s, int **edge, int *distance) {
    for(int i = 0; i < n; ++i)
        distance[i] = 0x3fffffff;
    distance[s] = 0;
    for(int i = 0; i < n-1; ++i) {
        for(int j = 0; j < m; ++j) {
            int a = edge[n][0];
            int b = edge[n][1];
            int w = edge[n][2];
            if(distance[a] + w < distance[b])
                distance[b] = distance[a] + w;
        }
    }
    for(int j = 0; j < m; ++j) {
        int a = edge[n][0];
        int b = edge[n][1];
        int w = edge[n][2];
        if(distance[a] + w < distance[b])
            return 0;
    }
    return 1;
}
