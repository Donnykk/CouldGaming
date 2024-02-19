# 云服务器资源调度

## 问题构建

常量：

- 服务器租用价格: $Price$
- 传输延迟阈值: $Latency$
- 带宽阈值: $Bandwidth$
- 工作负载阈值: $Workload$

对于客户端 $i$，服务中心 $j$

- 单位传输价格: $cost_i^j$
- 传输延迟: $l_i^j$
- 带宽: $b_i^j$

客户端集合: $\{c_1,...,c_n\}$

- 对于 $c_i$，屏幕切分成 s 个 Part: $p_1,...,p_q,...,p_s$
    每个 Part 占比: $prop_q$，有 $\sum\limits_{q=1}^{s}{prop_q} = 1$
    每个 Part 被分配到的服务中心 id 和服务器 id 记为 $ psc_i^q,ps_i^q $

- 客户端数据待渲染的工作负载记为 $wl_i$，单位时间内的数据传输量为 $d_i$

服务中心集合: $\{sc_1,...,sc_m\}$

- 对于 $sc_j$，租用服务器数量为 $num_j$，
    服务器集合 $\{s_j^1,..,s_j^{num_j}\}$，
- 对于服务器 $s_j^p$，当前工作负载记为 $w_{j}^p$

### 算法设计

每个客户端分配后的开销分为传输开销和服务器开销：
$$ TransCost = \sum\limits_{i=1}^n\sum\limits_{q=1}^s{prop_q*d_i*cost_i^{psc_i^q}} $$
$$ ServerCost = Price*\sum\limits_{j=1}^m{num_j} $$  
总成本即为：
$$ Cost = \sum\limits_{i=1}^n\sum\limits_{q=1}^s{prop_q*d_i*cost_i^{psc_i^q}}+Price*\sum\limits_{j=1}^m{num_j} $$

#### Lowest-Combined-Cost（LCC）

- 对于每一个客户端 $c_i$，遍历服务中心集合 $\{sc_1,...,sc_m\}$，根据延迟约束与带宽约束筛选出符合分配条件的服务中心集合 $SC$，$SC$ 按与当前客户端之间的单位传输价格升序排序
  - 约束条件：
    $$ \forall sc_j \in SC, \quad cost_i^j \leq Latency \quad \&\& \quad b_i^j < Bandwidth$$

- 遍历集合 $SC$ 查找第一个存在非空且可分配服务器的服务中心 $sc_a$,若不存在满足条件的服务中心，选用 $SC$ 中第一个（即单位传输价格最低）的服务中心

- 所选服务中心的首选可用服务器 $p$ 的剩余工作负载容量 $(Workload - w_a^p)$，若服务器剩余资源不足，对当前客户端屏幕待渲染资源进行切分，记分割屏幕比例为 $prop_q$，满足 $ wl_i * prop_q = Workload - w_a^p $，更新服务器状态为满载，回到步骤一继续分配

- 若客户端剩余待分配资源小于服务器剩余工作负载，则全部分配至当前服务器并更新服务器与服务中心状态

#### Hill Climbing（HC）

记 $ C_j $ 为对于服务中心 $sc_j$ 满足约束条件的客户端集合，即可分配至 $sc_j$ 的客户端集合

在每一轮分配过程中，考虑在每一个服务中心 $sc_j$ 开启一个新的服务器，并分配一组客户端 $ E_j \subseteq C_j $ 至该服务器。假设 $ C_j $ 依据传输数据量升序排序，那么 $ E_j $ 应为 $ C_j $ 的前n项，使得客户端平均成本最小：

$$ n = \argmin _{1 \leq m \leq |C_j|}{\frac{s(d)+\sum_{i=1}^m{cost_i^j * d_i}}{m}} $$
