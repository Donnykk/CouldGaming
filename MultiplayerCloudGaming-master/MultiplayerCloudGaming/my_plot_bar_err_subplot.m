%% cost
L_G = [75 150];
L_R = [50 100];
size = [10 50];
y_max = [3.5 1.8];

ss = get(0, 'ScreenSize');
set(gcf, 'Position', [ss(1) ss(2) ss(3) ss(4) * 2/3]);
for j = 1:2
   for i = 1:2
       file_name = sprintf('%d_%d_%d_costTotalMean', L_G(i), L_R(i), size(j));
       data_mean = importdata(strcat(file_name, '.csv'));
       data_mean = [
           data_mean(1, 2:end);
           data_mean(2, 2:end);
           data_mean(3, 2:end);
           data_mean(4, 2:end)];
       
       file_name_std = sprintf('%d_%d_%d_costTotalStd', L_G(i), L_R(i), size(j));
       data_std = importdata(strcat(file_name_std, '.csv'));
       data_std = [
           data_std(1, 2:end);
           data_std(2, 2:end);
           data_std(3, 2:end);
           data_std(4, 2:end)];       
             
       subplot(2, 2, (j - 1) * 2 + i);
       bh = barwitherr(data_std, data_mean); % with standard deviation bars       
            
       bh(1).FaceColor = rgb('FloralWhite');
       bh(2).FaceColor = rgb('LightBlue');
       bh(3).FaceColor = rgb('LightGreen');
       bh(4).FaceColor = rgb('Khaki');
       bh(5).FaceColor = rgb('LightPink');
       bh(6).FaceColor = rgb('SandyBrown');
       bh(7).FaceColor = rgb('MediumPurple');
      
       set(gca, 'XTick', [1 2 3 4]);
       set(gca, 'XTickLabel', [2 4 6 8]);
       set(gca, 'XLim', [0.5 4.5]);
       set(gca, 'YLim', [1 y_max(j)]);

       set(gca, 'fontsize', 14);
       xlabel('Server capacity: k', 'FontSize', 16);
       ylabel('Normalized cost', 'FontSize', 16);
       lh = legend('RA', 'NA', 'LSP', 'LBP', 'LCP', 'LCW', 'LAC', 'Orientation', 'vertical', 'Location', 'northwest');
       set(lh, 'FontSize', 12);       
       title(sprintf('(L_G = %d, L_R = %d); (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
       grid on;
    end 
end
export_fig cost.pdf -transparent

%% wastage
L_G = [75 150];
L_R = [50 100];
size = [10 50];
y_max = [4 1];

ss = get(0, 'ScreenSize');
set(gcf, 'Position', [ss(1) ss(2) ss(3) ss(4) * 2/3]);
for j = 1:2
   for i = 1:2              
       file_name = sprintf('%d_%d_%d_capacityWastageMean', L_G(i), L_R(i), size(j));
       data_mean = importdata(strcat(file_name, '.csv'));
       data_mean = [
           data_mean(1, 2:end);
           data_mean(2, 2:end);
           data_mean(3, 2:end);
           data_mean(4, 2:end)];
       
       file_name_std = sprintf('%d_%d_%d_capacityWastageStd', L_G(i), L_R(i), size(j));
       data_std = importdata(strcat(file_name_std, '.csv'));
       data_std = [
           data_std(1, 2:end);
           data_std(2, 2:end);
           data_std(3, 2:end);
           data_std(4, 2:end)];
        
       subplot(2, 2, (j - 1) * 2 + i);
       bh = barwitherr(data_std, data_mean);
       
       bh(1).FaceColor = rgb('FloralWhite');
       bh(2).FaceColor = rgb('LightBlue');
       bh(3).FaceColor = rgb('LightGreen');
       bh(4).FaceColor = rgb('Khaki');
       bh(5).FaceColor = rgb('LightPink');
       bh(6).FaceColor = rgb('SandyBrown');
       bh(7).FaceColor = rgb('MediumPurple');
      
      set(gca, 'XTick', [1 2 3 4]);
       set(gca, 'XTickLabel', [2 4 6 8]);
       set(gca, 'XLim', [0.5 4.5]);
       set(gca, 'YLim', [0 y_max(j)]);

       set(gca, 'fontsize', 14);
       xlabel('Server capacity: k', 'FontSize', 16);
       ylabel('Capacity wastage ratio', 'FontSize', 16); 
       lh = legend('RA', 'NA', 'LSP', 'LBP', 'LCP', 'LCW', 'LAC', 'Orientation', 'vertical', 'Location', 'northwest');
       set(lh, 'FontSize', 12); 
       title(sprintf('(L_G = %d, L_R = %d); (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
       grid on;
    end 
end
export_fig wastage.pdf -transparent