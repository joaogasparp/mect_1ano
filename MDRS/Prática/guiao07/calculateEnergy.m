function E = calculateEnergy(Links, Loads, L)
    E = 0;
    for i = 1:size(Links, 1)
        if Loads(i, 3) > 0 || Loads(i, 4) > 0
            % Link ativo
            E = E + 20.0 + 0.1 * L(Links(i, 1), Links(i, 2));
        else
            % Link em modo de espera
            E = E + 1.0;
        end
    end
end
