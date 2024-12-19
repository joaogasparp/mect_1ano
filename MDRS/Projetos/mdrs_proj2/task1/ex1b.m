%% 1.b.

clear
clc

fprintf('------------------------------ Task 1.b.------------------------------\n');

% carregar os dados
load('InputDataProject2.mat');

% parâmetros
nNodes = size(Nodes, 1);
nFlows = size(T, 1);
nLinks = size(Links, 1);

v = 2 * 10^5;
D = L / v;

anycastNodes = [3 10];

% inicializar variáveis para os atrasos e caminhos
Taux = zeros(nFlows, 4);
delays = zeros(nFlows, 1);

% calcular os caminhos mais curtos e os atrasos de ida e volta
for n = 1:nFlows
    if T(n, 1) == 1 || T(n, 1) == 2
        [shortestPath, totalCost] = kShortestPath(D, T(n, 2), T(n, 3), 1);
        sP{n} = shortestPath;
        nSP{n} = length(shortestPath);
        delays(n) = totalCost;
        Taux(n, :) = T(n, 2:5);
    elseif T(n, 1) == 3
        if ismember(T(n, 2), anycastNodes)
            sP{n} = {T(n, 2)};
            nSP{n} = 1;
            Taux(n, :) = T(n, 2:5);
            Taux(n, 3) = T(n, 2);
        else
            cost = inf;
            Taux(n, :) = T(n, 2:5);
            for i = anycastNodes
                [shortestPath, totalCost] = kShortestPath(D, T(n, 2), i, 1);
                if totalCost < cost
                    sP{n} = shortestPath;
                    nSP{n} = 1;
                    cost = totalCost;
                    delays(n) = totalCost;
                    Taux(n, 3) = i;
                end
            end
        end
    end
end

% calcular as cargas dos links
Loads = calculateLinkLoads(nNodes, Links, Taux, sP, ones(nFlows, 1));

% encontrar a pior carga de link
worstLinkLoad = max(max(Loads(:, 3:4)));

% mostrar os resultados
fprintf('Anycast nodes = %d  %d\n', anycastNodes(1), anycastNodes(2));
fprintf('Worst link load = %.2f Gbps\n', worstLinkLoad);
for i = 1:nLinks
    fprintf('{%d-%d}: %.2f %.2f\n', Loads(i, 1), Loads(i, 2), Loads(i, 3), Loads(i, 4));
end
