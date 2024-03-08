%% cost (k = 2)
L_G = [75 150];
L_R = [50 100];
size = [10 20 30 40 50];
k = [2 4 6 8];

i = 2;

cost_10 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(1)));
cost_20 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(2)));
cost_30 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(3)));
cost_40 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(4)));
cost_50 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(5)));

cost_k_2 = [
    cost_10(1, :); 
    cost_20(1, :); 
    cost_30(1, :); 
    cost_40(1, :); 
    cost_50(1, :)];

hold on; 
x = [10 20 30 40 50]; 
plot(x, cost_k_2(:, 3)./cost_k_2(:, 1), '-k^', 'LineWidth', 1, 'MarkerSize', 16);
plot(x, cost_k_2(:, 4)./cost_k_2(:, 1), '-ks', 'LineWidth', 1, 'MarkerSize', 16);
plot(x, cost_k_2(:, 7)./cost_k_2(:, 1), '-kx', 'LineWidth', 1, 'MarkerSize', 16);

set(gca, 'XTick', [10 20 30 40 50]);

set(gca, 'YLim', [1 1.5]); % for basic problem
%set(gca, 'YLim', [1 1.4]); % for general problem

set(gca, 'Box', 'on');
set(gca, 'fontsize', 24);
lh = legend('Nearest', 'LSP', 'LCW');
set(lh, 'FontSize', 24);
xlabel('Session size: |C|', 'FontSize', 30);
ylabel('Normalized cost', 'FontSize', 30);
title(sprintf('(L_G = %d, L_R = %d); \t (k = %d)', L_G(i)*2, L_R(i)*2, k(1)), 'fontsize', 24);

pbaspect([1.25 1 1]);
grid on;

file_name = sprintf('%d_%d_costTotalMean_vs_size_k_%d', L_G(i), L_R(i), k(1));
savefig(file_name);
print(file_name, '-dmeta');

%% cost (k = 8)
L_G = [75 150];
L_R = [50 100];
size = [10 20 30 40 50];
k = [2 4 6 8];

i = 2;

cost_10 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(1)));
cost_20 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(2)));
cost_30 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(3)));
cost_40 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(4)));
cost_50 = importdata(sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(5)));

cost_k_8 = [
    cost_10(4, :); 
    cost_20(4, :); 
    cost_30(4, :); 
    cost_40(4, :); 
    cost_50(4, :)];

hold on; 
x = [10 20 30 40 50]; 
plot(x, cost_k_8(:, 3)./cost_k_8(:, 1), '-k^', 'LineWidth', 1, 'MarkerSize', 16);
plot(x, cost_k_8(:, 4)./cost_k_8(:, 1), '-ks', 'LineWidth', 1, 'MarkerSize', 16);
plot(x, cost_k_8(:, 7)./cost_k_8(:, 1), '-kx', 'LineWidth', 1, 'MarkerSize', 16);

set(gca, 'XTick', [10 20 30 40 50]);

set(gca, 'YLim', [1 3.5]); % for basic problem
%set(gca, 'YLim', [1 2.01]); % for general problem

set(gca, 'Box', 'on');
set(gca, 'fontsize', 24);
lh = legend('Nearest', 'LSP', 'LCW');
set(lh, 'FontSize', 24);
xlabel('Session size: |C|', 'FontSize', 30);
ylabel('Normalized cost', 'FontSize', 30);
title(sprintf('(L_G = %d, L_R = %d); \t (k = %d)', L_G(i)*2, L_R(i)*2, k(4)), 'fontsize', 24);

pbaspect([1.25 1 1]);
grid on;

file_name = sprintf('%d_%d_costTotalMean_vs_size_k_%d', L_G(i), L_R(i), k(4));
savefig(file_name);
print(file_name, '-dmeta');

%% latency (k = 4)
L_G = [75 150];
L_R = [50 100];
size = [10 20 30 40 50];
y_min = [0 0];
k = [2 4 6 8];

i = 2;

cost_10 = importdata(sprintf('%d_%d_%d_2_averageLatencyMean', L_G(i), L_R(i), size(1)));
cost_20 = importdata(sprintf('%d_%d_%d_2_averageLatencyMean', L_G(i), L_R(i), size(2)));
cost_30 = importdata(sprintf('%d_%d_%d_2_averageLatencyMean', L_G(i), L_R(i), size(3)));
cost_40 = importdata(sprintf('%d_%d_%d_2_averageLatencyMean', L_G(i), L_R(i), size(4)));
cost_50 = importdata(sprintf('%d_%d_%d_2_averageLatencyMean', L_G(i), L_R(i), size(5)));

cost_k_4 = [
    cost_10(2, :);
    cost_20(2, :); 
    cost_30(2, :); 
    cost_40(2, :);
    cost_50(2, :)];

hold on; 
x = [10 20 30 40 50]; 
plot(x, cost_k_4(:, 3).*2, '-k^', 'LineWidth', 1, 'MarkerSize', 16);
plot(x, cost_k_4(:, 4).*2, '-ks', 'LineWidth', 1, 'MarkerSize', 16);
plot(x, cost_k_4(:, 7).*2, '-kx', 'LineWidth', 1, 'MarkerSize', 16);

set(gca, 'XTick', [10 20 30 40 50]);
set(gca, 'YLim', [y_min(i) L_G(i)*2]);

set(gca, 'Box', 'on');
set(gca, 'fontsize', 24);
lh = legend('Nearest', 'LSP', 'LCW', 'location', 'southeast');
set(lh, 'FontSize', 24);
xlabel('Session size: |C|', 'FontSize', 30);
ylabel('Average latency (ms)', 'FontSize', 30);
title(sprintf('L_G = %d; \t k = %d', L_G(i)*2, k(2)), 'fontsize', 24);

pbaspect([1.5 1 1]);
grid on;

file_name = sprintf('%d_%d_averageLatencyMean_vs_size_k_%d', L_G(i), L_R(i), k(2));
savefig(file_name);
print(file_name, '-dmeta');