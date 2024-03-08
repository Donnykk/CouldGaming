%% grouping rate
set(gcf, 'Position', get(0, 'Screensize'));
L_T = [25 50 100];
S_S = [10 20 40];
for i = 1:length(L_T)
    %eligible
    subplot(3, 4, (i - 1) * 4 + 1);
    alg = importdata(sprintf('layered_%d_%d.csv', L_T(i), S_S(1)));
    plot(alg(:, 1), alg(:, 2), '-xk', 'LineWidth', 2);
    set(gca, 'YLim', [0 1], 'fontsize', 10);
    xlabel('number of clients in the lobby', 'fontsize', 12);
    ylabel('ratio of eligible over total', 'fontsize', 12);
    title(sprintf('latencyThreshold = %d', L_T(i)));
    grid on;
    box on;
    pbaspect([1.5 1 1]);
    
    %algorithm
    for j = 1:length(S_S)
        subplot(3, 4, (i - 1) * 4 + 1 + j);        
       
        alg = importdata(sprintf('layered_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 3)./alg(:, 2), '-*', 'LineWidth', 0.5);
        hold on;
        alg = importdata(sprintf('simple_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 3)./alg(:, 2), '-s', 'LineWidth', 0.5);
        alg = importdata(sprintf('nearest_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 3)./alg(:, 2), '-^', 'LineWidth', 0.5);
        alg = importdata(sprintf('random_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 3)./alg(:, 2), '-o', 'LineWidth', 0.5);
       
        %if i == 1 && j == 1
            lg = legend('layered', 'simple', 'nearest', 'random', 'Location', 'southeast');
            set(lg, 'FontSize', 10);
        %end
        
        set(gca, 'YLim', [0 1], 'fontsize', 10);
        xlabel('number of clients in the lobby', 'fontsize', 12);
        ylabel('grouping rate (over eligible)', 'fontsize', 12);
        title(sprintf('latencyThreshold = %d | sessionSize = %d', L_T(i), S_S(j)));
        grid on;
        box on;
        pbaspect([1.5 1 1]);
    end
end
export_fig grouping_rate.pdf -transparent

%% grouping time
set(gcf, 'Position', get(0, 'Screensize'));
L_T = [25 50 100];
S_S = [10 20 40];
for i = 1:3
    %eligible
    subplot(3, 4, (i - 1) * 4 + 1);
    alg = importdata(sprintf('layered_%d_%d.csv', L_T(i), S_S(1)));
    plot(alg(:, 1), alg(:, 2), '-xk', 'LineWidth', 2);
    set(gca, 'YLim', [0 1], 'fontsize', 10);
    xlabel('total number of clients', 'fontsize', 12);
    ylabel('ratio of eligible over total', 'fontsize', 12);
    title(sprintf('latencyThreshold = %d', L_T(i)));
    grid on;
    box on;
    pbaspect([1.5 1 1]);
    
    %algorithm
    for j = 1:3
        subplot(3, 4, (i - 1) * 4 + 1 + j);        
       
        alg = importdata(sprintf('layered_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 4), '-*', 'LineWidth', 0.5);
        hold on;
        alg = importdata(sprintf('simple_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 4), '-s', 'LineWidth', 0.5);
        alg = importdata(sprintf('nearest_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 4), '-^', 'LineWidth', 0.5);
        alg = importdata(sprintf('random_%d_%d.csv', L_T(i), S_S(j)));
        plot(alg(:, 1), alg(:, 4), '-o', 'LineWidth', 0.5);
       
        %if i == 1 && j == 1
            lg = legend('layered', 'simple', 'nearest', 'random', 'Location', 'northwest');
            set(lg, 'FontSize', 10);
        %end
        
        set(gca, 'fontsize', 10);
        set(gca, 'yscale', 'log');
        xlabel('number of clients in the lobby', 'fontsize', 12);
        ylabel('grouping time (msec)', 'fontsize', 12);
        title(sprintf('latencyThreshold = %d | sessionSize = %d', L_T(i), S_S(j)));
        grid on;
        box on;
        pbaspect([1.5 1 1]);
    end
end
export_fig grouping_time.pdf -transparent