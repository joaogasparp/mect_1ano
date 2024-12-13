function [sol, load, Loads] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP)
    nFlows = size(T, 1);
    sol = zeros(1, nFlows);
    bestLoad = inf;
    bestLoads = [];

    for f = 1:nFlows
        bestPathLoad = inf;
        for p = 1:nSP(f)
            sol(f) = p;
            Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);
            load = max(max(Loads(:, 3:4)));
            if load < bestPathLoad
                bestPathLoad = load;
                bestPath = p;
            end
        end
        sol(f) = bestPath;
    end

    Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);
    load = max(max(Loads(:, 3:4)));
end
