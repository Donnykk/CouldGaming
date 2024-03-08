%% cost
L_G = [75 150];
L_R = [50 100];
size = [10 50];

y_max = [4.5 2]; % for basic problem
%y_max = [4 2]; % for general problem

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
           data_std(4, 2:end)
           ];
       
       figure;
       
       %bh = bar(data_mean);
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
       set(gca, 'YLim', [1 y_max(j)]);

       set(gca, 'fontsize', 16);
       xlabel('Server capacity: k', 'FontSize', 20);
       ylabel('Normalized cost', 'FontSize', 20);
       lh = legend('RA', 'NA', 'LSP', 'LBP', 'LCP', 'LCW', 'LAC', 'Orientation', 'horizontal', 'Location', 'northoutside');
       set(lh, 'FontSize', 12);       
       title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
       grid on;       
       pbaspect([2.5 1 1]);
       %pbaspect([2 1 1]);
       
       savefig(file_name);
       print(file_name, '-dmeta');
    end 
end

%% wastage
L_G = [75 150];
L_R = [50 100];
size = [10 50];

y_max = [5 1]; % for basic problem
%y_max = [4 1]; % for general problem

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
           data_std(4, 2:end)
           ];
 
       figure;
       
       %bh = bar(data_mean);
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
       set(gca, 'YLim', [0 y_max(j)]);

       set(gca, 'fontsize', 16);
       xlabel('Server capacity: k', 'FontSize', 20);
       ylabel('Capacity wastage ratio', 'FontSize', 20); 
       lh = legend('RA', 'NA', 'LSP', 'LBP', 'LCP', 'LCW', 'LAC', 'Orientation', 'horizontal', 'Location', 'north');
       set(lh, 'FontSize', 12); 
       title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
       grid on;       
       pbaspect([2.5 1 1]);
       %pbaspect([2 1 1]);      
       
       savefig(file_name);
       print(file_name, '-dmeta');
    end 
end