function [sol, load] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load)
    nFlows = size(T,1);    
    % definir as melhores variáveis locais
    bestLocalLoad = load;
    bestLocalSol = sol;

    % hill climbing
    improved = true;
    while improved
    
        % testar cada fluxo
        for flow = 1 : nFlows
            % testar cada caminho do fluxo
            for path = 1 : nSP(flow)
                if path ~= sol(flow)

                    % alterar o caminho para esse fluxo
                    auxSol = sol;
                    auxSol(flow) = path;

                    % calcular as cargas
                    Loads = calculateLinkLoads(nNodes, Links, T, sP, auxSol);
                    auxLoad = max(max(Loads(:, 3:4)));
                        
                    % verificar se a carga atual é melhor que a carga inicial
                    if auxLoad < bestLocalLoad
                        bestLocalLoad = auxLoad;
                        bestLocalSol = auxSol;
                    end
                end
            end
        end

        if bestLocalLoad < load
            load = bestLocalLoad;
            sol = bestLocalSol;
        else
            improved = false;
        end
    end
end