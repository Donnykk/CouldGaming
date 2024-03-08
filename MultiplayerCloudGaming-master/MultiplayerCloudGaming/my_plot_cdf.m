%% eligible RDatacenter count cdf (basic problem) seperate session sizes (10 and 50 only)
L_G = [75 150];
L_R = [50 100];

legend_name = repmat('', 2, 1);
line_style = {'-', ':'};

ss = get(0, 'ScreenSize');
set(gcf, 'Position', [ss(1) ss(2) ss(3)/2 ss(4)/3]);
pos = 0;
for size = [10 50]
    pos = pos + 1;
    subplot(1, 2, pos);
    for i = 1:2
        data = importdata(sprintf('%d_%d_%d_eligibleRDatacenterCount.csv', L_G(i), L_R(i), size));
        p_h = cdfplot(data);    
        hold on;
        set(p_h, 'LineStyle', line_style{i}, 'LineWidth', 3, 'Color', 'k'); 
        legend_name{i} = sprintf('(L_G = %d, L_R = %d)', L_G(i)*2, L_R(i)*2);
    end
    set(gca, 'XLim', [0 10]);
    set(gca, 'XTick', [0 1 2 3 4 5 6 7 8 9 10]); 
    set(gca, 'fontsize', 14);
    lh = legend(legend_name, 'Orientation', 'vertical', 'Location', 'best');
    set(lh, 'FontSize', 12);
    xlabel('Number of eligible datacenters per client', 'FontSize', 14);
    ylabel('Cumulative distribution function', 'FontSize', 14);   
    title(sprintf('|C| = %d', size));
    grid on;
    hold off;
end
export_fig cdf_rs_basic.pdf -transparent

%% eligible GDatacenter count cdf (general problem) seperate session sizes (10 and 50 only)
L_G = [75 150];
L_R = [50 100];

legend_name = repmat('', 2, 1);
line_style = {'-', ':'};

ss = get(0, 'ScreenSize');
set(gcf, 'Position', [ss(1) ss(2) ss(3)/2 ss(4)/3]);
pos = 0;
for size = [10 50]
    pos = pos + 1;
    subplot(1, 2, pos);
    for i = 1:2
        data = importdata(sprintf('%d_%d_%d_eligibleGDatacenterCount.csv', L_G(i), L_R(i), size));
        p_h = cdfplot(data);    
        hold on;
        set(p_h, 'LineStyle', line_style{i}, 'LineWidth', 3, 'Color', 'k'); 
        legend_name{i} = sprintf('(L_G = %d, L_R = %d)', L_G(i)*2, L_R(i)*2);
    end
    set(gca, 'XLim', [0 13]);
    set(gca, 'XTick', [0 1 2 3 4 5 6 7 8 9 10 11 12 13]);    
    set(gca, 'fontsize', 14);
    lh = legend(legend_name, 'Orientation', 'vertical', 'Location', 'best');
    set(lh, 'FontSize', 12);
    xlabel('Number of eligible datacenters for the G-server', 'FontSize', 14);
    ylabel('Cumulative distribution function', 'FontSize', 14);
    title(sprintf('|C| = %d', size));
    grid on;   
    hold off;
end
export_fig cdf_gs_general.pdf -transparent