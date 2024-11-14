%% 8.a.

fprintf('Executing 8.a...\n');
clear all
close all
clc

load('InputData.mat')
nNodes = size(Nodes, 1);
nLinks = size(Links, 1);
nFlows= size(T,1);

k = 1;
sP = cell(1,nFlows);
nSP = zeros(1,nFlows);
for f = 1 : nFlows
    [shortestPath, totalCost] = kShortestPath(L, T(f, 1), T(f, 2), k);
    sP{f} = shortestPath;
    nSP(f) = totalCost;
    fprintf('Flow %d (%d -> %d): length = %d, Path = %s\n', f, T(f, 1), T(f, 2), totalCost, num2str(shortestPath{1}));
end

%% 8.b.

fprintf('Executing 8.b...\n');

sol = ones(1, nFlows);
Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);

maxLoad = max(max(Loads(:,3:4)));
fprintf('Worst link load = %.2f\n', maxLoad);

for i = 1 : length(Loads)
    fprintf('{%d-%d}:\t%.2f\t%.2f\n', Loads(i, 1), Loads(i, 2), Loads(i, 3), Loads(i, 4));
end

%% 8.c.

fprintf('Executing 8.c...\n');

k = 4;
f = 1;
[shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);

for i = 1 : length(totalCost)
    fprintf('Path %d = %s (length= %d)\n',i,num2str(shortestPath{i}),totalCost(i));
end

%% 8.d.

fprintf('Executing 8.d...\n');
clear all
close all
clc

load('InputData.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

k= inf;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end

sol= ones(1,nFlows);
Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
maxLoad= max(max(Loads(:,3:4)));

t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    sol= zeros(1,nFlows);
    for f= 1:nFlows
        sol(f)= randi(nSP(f));
    end
    Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
    load= max(max(Loads(:,3:4)));
    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
    end
    contador= contador+1;
    somador= somador+load;
end

fprintf('Routing paths of the solution:\n')
for f= 1:nFlows
    selectedPath= bestSol(f);
    fprintf('Flow %d - Path %d:  %s\n',f,selectedPath,num2str(sP{f}{selectedPath}));
end

fprintf('Worst link load of the best solution = %.2f\n',bestLoad);
fprintf('Link loads of the best solution:\n')
for i= 1:nLinks
    fprintf('{%d-%d}:\t%.2f\t%.2f\n',bestLoads(i,1),bestLoads(i,2),bestLoads(i,3),bestLoads(i,4))
end

fprintf('No. of generated solutions = %d\n',contador);
fprintf('Avg. worst link load among all solutions= %.2f\n',somador/contador);

% Os resultados obtidos são diferentes dos obtidos anteriormente.
% A carga máxima dos links é significativamente melhor em comparação 
% com a solução anterior, mas a carga média máxima dos links para
% todas as soluções geradas é próxima do valor obtido anteriormente.

%% 8.e.

fprintf('Executing 8.e...\n');
clear all
close all

load('InputData.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

k= 6;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end

sol= ones(1,nFlows);
Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
maxLoad= max(max(Loads(:,3:4)));

t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    sol= zeros(1,nFlows);
    for f= 1:nFlows
        sol(f)= randi(nSP(f));
    end
    Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
    load= max(max(Loads(:,3:4)));
    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
    end
    contador= contador+1;
    somador= somador+load;
end

fprintf('\nRouting paths of the solution:\n')
for f= 1:nFlows
    selectedPath= bestSol(f);
    fprintf('Flow %d - Path %d:  %s\n',f,selectedPath,num2str(sP{f}{selectedPath}));
end

fprintf('Worst link load of the best solution = %.2f\n',bestLoad);
fprintf('Link loads of the best solution:\n')
for i= 1:nLinks
    fprintf('{%d-%d}:\t%.2f\t%.2f\n',bestLoads(i,1),bestLoads(i,2),bestLoads(i,3),bestLoads(i,4))
end

fprintf('No. of generated solutions = %d\n',contador);
fprintf('Avg. worst link load among all solutions= %.2f\n',somador/contador);

% Obtemos uma rede diferente, mas com um valor de carga máxima dos 
% links igual à solução anterior.

%% 8.f.

% Implementar um algoritmo baseado na estratégia greedy randomized

%% 8.g.

fprintf('Executing 8.g...\n');
clear all
close all

load('InputData.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

k= 6;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end

sol= ones(1,nFlows);
Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
maxLoad= max(max(Loads(:,3:4)));

t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    [sol, load] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP);

    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
    end
    contador= contador+1;
    somador= somador+load;
end

fprintf('\nRouting paths of the solution:\n')
for f= 1:nFlows
    selectedPath= bestSol(f);
    fprintf('Flow %d - Path %d:  %s\n',f,selectedPath,num2str(sP{f}{selectedPath}));
end

fprintf('Worst link load of the best solution = %.2f\n',bestLoad);
fprintf('Link loads of the best solution:\n')
for i= 1:nLinks
    fprintf('{%d-%d}:\t%.2f\t%.2f\n',bestLoads(i,1),bestLoads(i,2),bestLoads(i,3),bestLoads(i,4))
end

fprintf('No. of generated solutions = %d\n',contador);
fprintf('Avg. worst link load among all solutions= %.2f\n',somador/contador);

% O algoritmo greedy randomized é muito mais pesado que o
% algoritmo random (gera menos soluções), mas as soluções
% geradas (em média) são muito melhores.

%% 8.h.

fprintf('Executing 8.h...\n');
clear all
close all

load('InputData.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

k= inf;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end

sol= ones(1,nFlows);
Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
maxLoad= max(max(Loads(:,3:4)));

t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    [sol, load] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP);

    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
    end
    contador= contador+1;
    somador= somador+load;
end

fprintf('\nRouting paths of the solution:\n')
for f= 1:nFlows
    selectedPath= bestSol(f);
    fprintf('Flow %d - Path %d:  %s\n',f,selectedPath,num2str(sP{f}{selectedPath}));
end

fprintf('Worst link load of the best solution = %.2f\n',bestLoad);
fprintf('Link loads of the best solution:\n')
for i= 1%nLinks
    fprintf('{%d-%d}:\t%.2f\t%.2f\n',bestLoads(i,1),bestLoads(i,2),bestLoads(i,3),bestLoads(i,4))
end

fprintf('No. of generated solutions = %d\n',contador);
fprintf('Avg. worst link load among all solutions= %.2f\n',somador/contador);
