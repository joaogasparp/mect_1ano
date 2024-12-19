function [sol, load] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP)
    nFlows = size(T,1);
    % ordem aleatória dos fluxos
    randFlows = randperm(nFlows);
    sol = zeros(1, nFlows);

    % iterar por cada fluxo
    for flow = randFlows
        path_index = 0;
        best_load = inf;

        % testar cada caminho "possível" com uma certa carga
        for path = 1 : nSP(flow)
            % tentar o caminho para esse fluxo
            sol(flow) = path;
            % calcular as cargas
            Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);
            load = max(max(Loads(:, 3:4)));
            
            % verificar se a carga atual é melhor que a melhor carga
            if load < best_load
                % change index of path and load
                path_index = path;
                best_load = load;
            end
        end
        sol(flow) = path_index;
    end
    load = best_load;
end