%% 9.b.

clear
clc

load('InputData2.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

% Computing up to k=inf shortest paths for all flows from 1 to nFlows:
k= inf;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end
% sP{f}{i} is the i-th path of flow f
% nSP(f) is the number of paths of flow f

% Compute the link loads using the first (shortest) path of each flow:
sol= ones(1,nFlows);
Loads= calculateLinkLoads(nNodes,Links,T,sP,sol);
% Determine the worst link load:
maxLoad= max(max(Loads(:,3:4)));

% print the minimum no. of paths
fprintf('Minimum no. of paths= %d\n', min(nSP));
for flow = find(nSP==min(nSP))
    fprintf('\tFlow %d (%d -> %d)\n', flow, sP{flow}{sol(flow)}(1), sP{flow}{sol(flow)}(end));
end

% print the maximum no. of paths
fprintf('\nMaximum no. of paths= %d\n', max(nSP));
for flow = find(nSP==max(nSP))
    fprintf('\tFlow %d (%d -> %d)\n', flow, sP{flow}{sol(flow)}(1), sP{flow}{sol(flow)}(end));
end

%% 9.c.

clear
clc

load('InputData2.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

% Computing up to k=inf shortest paths for all flows from 1 to nFlows:
k= inf;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end
% sP{f}{i} is the i-th path of flow f
% nSP(f) is the number of paths of flow f

%%%%%%%%%%%%%%%%%%%%%%%%%%%% Random Algorithm %%%%%%%%%%%%%%%%%%%%%%%%%%%%
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
        bestLoadTime = toc(t);
    end
    contador= contador+1;
    somador= somador+load;
end

fprintf('Random algorithm (all possible paths):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);

%%%%%%%%%%%%%%%%%%%%%% Greedy Randomized Algorithm %%%%%%%%%%%%%%%%%%%%%%%
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
        bestLoadTime = toc(t);
    end
    contador= contador + 1;
    somador= somador + load;
end

fprintf('Greedy randomized (all possible paths):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);

%%%%%%%%%%%%%%%%% Multi start hill climbing with random %%%%%%%%%%%%%%%%%%
t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    % randomized start
    sol = zeros(1,nFlows);
    for f= 1:nFlows
        sol(f)= randi(nSP(f));
    end
    
    Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);
    load = max(max(Loads(:, 3:4)));

    [sol, load] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load);

    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
        bestLoadTime = toc(t);
    end
    contador= contador + 1;
    somador= somador + load;
end

fprintf('Multi start hill climbing with random (all possible paths):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);

%%%%%%%%%%% Multi start hill climbing with  greedy randomized %%%%%%%%%%%%
t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    % greedy randomzied start
    [sol, load] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP);

    [sol, load] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load);

    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
        bestLoadTime = toc(t);
    end
    contador= contador + 1;
    somador= somador + load;
end

fprintf('Multi start hill climbing with greedy randomized (all possible paths):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);

%% 9.d.

clear
clc

load('InputData2.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

% Computing up to k=6 shortest paths for all flows from 1 to nFlows:
k= 6;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end
% sP{f}{i} is the i-th path of flow f
% nSP(f) is the number of paths of flow f

%%%%%%%%%%%%%%%%%%%%%%%%%%%% Random Algorithm %%%%%%%%%%%%%%%%%%%%%%%%%%%%
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
        bestLoadTime = toc(t);
    end
    contador= contador+1;
    somador= somador+load;
end

fprintf('Random algorithm (6 paths per flow):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);
% Random algorithm (6 paths per flow):
% W = 7.30 Gbps, No. sol = 417312, Av. W = 12.82, time = 1.78 sec

%%%%%%%%%%%%%%%%%%%%%% Greedy Randomized Algorithm %%%%%%%%%%%%%%%%%%%%%%%
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
        bestLoadTime = toc(t);
    end
    contador= contador + 1;
    somador= somador + load;
end

fprintf('Greedy randomized algorithm (6 paths per flow):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);
% Greedy randomized algorithm (6 paths per flow):
% W = 6.70 Gbps, No. sol = 8040, Av. W = 7.91, time = 0.30 sec

%%%%%%%%%%%%%%%%% Multi start hill climbing with random %%%%%%%%%%%%%%%%%%
t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    % randomized start
    sol = zeros(1,nFlows);
    for f= 1:nFlows
        sol(f)= randi(nSP(f));
    end
    
    Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);
    load = max(max(Loads(:, 3:4)));

    [sol, load] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load);

    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
        bestLoadTime = toc(t);
    end
    contador= contador + 1;
    somador= somador + load;
end

fprintf('Multi start hill climbing with random (6 paths per flow):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);
% Multi start hill climbing with random (6 paths per flow):
% W = 6.70 Gbps, No. sol = 1041, Av. W = 7.89, time = 0.36 sec

%%%%%%%%%%% Multi start hill climbing with  greedy randomized %%%%%%%%%%%%
t= tic;
timeLimit= 5;
bestLoad= inf;
contador= 0;
somador= 0;
while toc(t) < timeLimit
    % greedy randomzied start
    [sol, load] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP);

    [sol, load] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load);

    if load<bestLoad
        bestSol= sol;
        bestLoad= load;
        bestLoads= Loads;
        bestLoadTime = toc(t);
    end
    contador= contador + 1;
end

fprintf('Multi start hill climbing with greedy randomized (6 paths per flow):\n');
fprintf('\t W = %.2f Gbps, No. sol = %d, Av. W = %.2f, time = %.2f sec\n', bestLoad, contador, somador/contador, bestLoadTime);
% Multi start hill climbing with greedy randomized (6 paths per flow):
% W = 6.70 Gbps, No. sol = 2409, Av. W = 7.47, time = 0.17 sec
