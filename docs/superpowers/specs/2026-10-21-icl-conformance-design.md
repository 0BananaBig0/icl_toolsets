# ICL 逐块一致性审查与修正设计

状态：用户于 2026-10-22 以 “Proceed” 确认。此文档描述整项交付，具体修改清单在逐块审查后确定。

## 目标与约束

依据 IEEE Std 1687-2014 的规范正文逐块审查并修正本仓库的 ICL Flex/Bison 文法、正则、相关手写 C++ 和测试。顺序从 Enum 开始，完成一个块的证据、修正和验证，再进入下一个块；不能把“全部先改完”当作逐块验收。

ICL 和 PDL 是两种语言。本仓库当前只有 `src/icl_lexer.l` 和 `src/icl_parser.yy`，没有 PDL Flex/Bison 实现，因此不新增 PDL 解析器。保留 ICL 对 PDL 命令、数字和注释的隔离测试。ICL 中规范规定的 iProc 回调引用仍属于 ICL 语法，不因名称包含 iProc 就删除。

`icl-format` 与 `icl-lsp` 必须继续共享 AST 定义及实际处理链。命令行参数与 LSP JSON 协议、输出包装可以不同。

用户要求最终运行安装命令，并打印两个工具的 coc.nvim 使用配置。此请求不包含修改用户已有编辑器配置或发布 Git 远端。

## 已确认的现状

- 工作树初始干净，起始提交 `44fc3db`。
- `src/icl_ast.h` 是唯一 AST 定义；两个前端调用同一个 `FormatDocument`，共用 Parse → AST → BuildFormatTree → RenderFormatTree。保留这一结构。
- 文法大量复用 `attr_body`、通用表达式和外形分组，导致块内成员和嵌套位置缺乏约束。现有 block_review 中存在以实现可接受范围为依据的正例，必须重新核实。
- IEEE §6.5.3 的 `enum_item+` 要求非空枚举，现有通用块体允许空块；这将成为 Enum 阶段首先验证的反例。
- 扫描器的标识符、数值、字符串规则与 IEEE 的逐项一致性尚未通过完整审查。
- `tests/coverage.py` 会跳过缺失的源报告，且忽略部分没有计数的分支；“高覆盖率”结论需重新建立。`grammar_coverage.py` 按源行去重，需核实同一行多条产生式是否被掩盖。
- Honza255/icl_parser 已固定到 `333f8bc7a54964138ac14a34fde9e942c496436d`。其 8 份 `.icl` 测试输入全部有逐字节一致的本地副本，Python 内嵌输入、关键字与产生式覆盖尚待完成。
- 初步字符串清单对照显示，上游 ICL 文法中的 74 个多字符关键字／固定字面量全部有本地显式 lexer 规则；这不证明它们在正确语法位置具有完整匹配能力。关键字表“没有缺项”不能替代产生式对照。
- 两份用户 merged ICL 位于 `tests/cases/pass/tessent/`，与既有 CHECKSUMS.md 一致，大小为 141671 与 137377 字节。
- 已使用 `/usr/bin/bison` 3.8.2、Flex 2.6.4、GCC 16.2.1 构建两个动态程序；默认 PATH 中 Bison 3.3 不满足项目要求。基线 `tests/regression.py` 为 494 项通过；其他检查尚未执行，不能声称全套通过。
- 随后运行完整 `make -j2 check BISON=/usr/bin/bison`，在静态链接阶段因缺少 libstdc++、libm、libc 静态库而失败，尚未进入该目标的完整测试步骤。不能把这一结果报告为全套回归通过。
- 没有 CodeGraph 索引，已回退到源码检索，不自动创建索引。

## 文法兼容策略（待确认的设计选择）

建议以 IEEE 1687-2014 为基础，保留能够由真实用户文件或可定位的第三方／Tessent 文档证明的扩展；在用例和支持表中明确标为扩展。不能用一个任意块体兜底来保留所有未知语法，也不能把第三方解析器接受等同于标准合法。

这一策略会使过去误接受的非标准输入开始报错。对于 Reg、Field、旧式 mux、Origen include 等既有形式，逐一记录来源后决定保留的精确范围；无证据的自造正例不得继续充当支持理由。用户对兼容策略的选择优先于此建议。

不添加未经请求的方言模式开关。若用户选择严格标准或全部保留，则在计划开始前调整本节及对应验收标准，避免混合两个目标。

语义范围：覆盖规范中影响词法、产生式、块内允许成员、出现次数及局部语法约束的规则。此项目仍是 formatter 和格式化 LSP，不在本次扩展为完整 IJTAG elaborator、连接可达性求解器或硅上执行器；需要跨模块电路求解才能判定的约束明确记录为未实现，不能宣称完整硬件 DRC 合规。

## 修改方式与共享架构

优先在现有 `.l`、`.yy` 和共享层修正根因。针对实际不同的语法块定义对应产生式，只共享确实具有相同语义的词法／产生式片段。不能因为布局相同就允许相同的内部语法。

AST 保留统一所有权和布局模型。若某项局部检查确需额外结构化信息，只在共享 AST／parser 层增加最少信息；不建立 CLI 专用 AST 或 LSP 专用 parser。不为每个关键字机械创建类。

保持共同链路：

`CLI 参数 / LSP 参数 → FormatRequest → FormatDocument → scanner/parser → 同一 AST → format tree → render → FormatOutcome → CLI 输出 / LSP TextEdit 与诊断`

错误内容、样式归一化、是否有改动等可共享行为继续留在公共层。LSP 的诊断请求若存在可证明的重复计算，再依据测量决定是否优化；不预先增加缓存或并发状态。审查所有调用者后再移除冗余接口、临时字符串或死分支，不为覆盖率删除必要错误处理。

## 串行块清单与每块完成条件

依次处理：

1. Enum（§6.5.3）及它需要的标识符、数字、拼接／取反和注释。
2. DataOutPort（§6.4.6.7）；随后逐个处理 DataInPort、ScanInPort、ScanOutPort，以及全部控制／时钟／复位端口，分别核查成员与可选形式。
3. ScanRegister、DataRegister，包括 Selectable、Addressable、回调和嵌套。
4. LogicSignal、ScanMux、DataMux、ClockMux，包括表达式、操作符、列表和递归。
5. OneHotScanGroup、OneHotDataGroup、ScanInterface 与 Chain。
6. Instance、AccessLink 及 1149.1／通用形式。
7. Alias、Parameter、LocalParameter、Attribute、NameSpace、UseNameSpace、Module／文件结构。
8. 交叉检查全部共享词法、关键字作标识符、字符串、注释位置、数值边界、递归深度及已证实的扩展，填补前面未触及的分支。

每块维护一行或一组审查记录：IEEE 条款和页码 → 原始正则／产生式 → 上游对应规则 → 本地对应规则 → 正反例／递归例 → 修正 → 实际检查结果。状态必须区分未审查、已发现问题、已修正、已验证。

每块先加入能重现差异的测试，观察当前实现的失败，再修正共享代码。完成该块的正例、反例、边界、格式化幂等、内容保持以及与此前已确认块的回归后，再进入下一块。公共规则改动必须回归此前依赖它的块。

## 测试组织与外部对照

沿用仓库实际测试目录 `tests/`（即用户所述的 test 目录），不另建平行 `test/`。新增测试脚本按被测源码相对路径放在 `tests/src/` 等对应位置；既有脚本可原地维护。新用例按来源先分类，再按块分类，例如 `tests/cases/standard/enum/`、`tests/cases/tessent/data_out_port/`、`tests/cases/upstream/enum/`、`tests/cases/ours/enum/`，用清单明确期望接受／拒绝，不靠目录存在与否推断结果。现有用例保持原位并纳入统一运行。

所有用于验证的输入都保存在仓库测试目录；运行时生成的随机输入要记录 seed，可缩减的失败保存为分类回归输入。把新增或需修正的内嵌 block_review 输入提取为可追溯用例。每个规范／Tessent 片段标明原始定位与为了成为完整 ICL 文件新增的外壳、声明及连接；不把补全内容冒充原文。

Honza255 对照分两层：

- 关键字：排除数字字母和 PDL token，按 ICL 语法位置检查本地是否具有真正的对应匹配，而非只检查 lexer 中是否出现一个字符串。
- 匹配能力：逐产生式比较分支、可选项、重复、嵌套和边界；对“它接受／我们拒绝”和反向差异查 IEEE 原文，形成有来源的结论。

其 Python 数值对象的计算／电路模拟测试只提取本项目有对应职责的源文本行为，不能把没有数值求值功能当作 formatter 缺陷。差分调用必须同时计入 lexer 错误并检查完整输入消费；不能把参考 parser 未消费的尾部当作有效输入。外部依赖缺失时明确标为未运行，不混入通过数。

参考公开 parser、formatter、LSP 项目和 Flex/Bison 文档的实践，记录采用的具体方法与来源：接受／拒绝、golden 输出、注释与 token 保持、重复格式化稳定性、源定位、逐产生式 trace、lexer 规则命中、协议 framing／UTF-16 位置及错误后恢复。外部语料记录 revision、原路径、必要修改和许可。

## 覆盖率与正确性验收

两种覆盖率分开报告：

1. `.yy` 的每条用户产生式、`.l` 的每条用户扫描规则的命中情况，不能用“一个源行被碰到”代替每条产生式。
2. 手写 C++、头文件内实际逻辑，以及 `.yy/.l` 中手写 action／辅助代码的行和可执行分支覆盖。Bison/Flex 自动生成实现不作为目标。

修正 gcov 读取器，对缺失报告、空分母、零次／never-executed 分支和未计数行采用明确处理。只排除有证据的生成器／编译器产物，并说明口径，不隐去真实未覆盖错误路径。

对手写逻辑以全行和尽可能全分支为目标；现有 100% 行／90% 分支门槛先核实其真实分母，再保持或提高。不能预先承诺所有故障注入路径都能到 100%；剩余分支逐项说明原因和风险，不以百分比掩盖空跑或错误期待。

完整收尾检查包括动态／静态构建、所有本地测试、修正后的 coverage 和 grammar coverage、独立代码审查、安装验证，以及可用的 coc.nvim 真实集成测试。新增测试不得只镜像实现或仅检查源代码含有某段字符串。

## 性能与规模

基准采用两份真实 merged ICL、现有大型 benchmark，以及可重现的多模块扩展输入／深度嵌套边界。记录字节数、模块数或深度、工具链、命令、墙钟时间、峰值 RSS；CLI 与 LSP 请求各选有代表性的输入。测量在原始实现与最终实现上使用相同参数，重复短耗时场景以降低启动抖动。

目标是不引入不必要的重复解析、每 token 子进程或二次方拼接退化；无明确时延预算时以现有基线及扩展趋势评估，不声称未经测量的加速。发现性能退化时先定位共享路径再选择最小修正。必要的正确性成本需要明确报告。

## 安装与交付

默认采用已有 `make install-dynamic`，安装到 `$HOME/.local`；同时构建验证静态变体，若当前环境缺少静态库则明确报告并按需解决，不假装成功。安装前审查目标和 manifest，安装后验证实际路径、版本及基本行为；越出沙箱的写入通过平台审批执行。

交付包含：逐块核对记录、关键字／匹配差异表、来源清单、正确性和覆盖率结果、性能测量、未解决限制。更新 README 中与实际行为不符的合规与宽松语法说明，避免继续引用历史结果充当本次证据。

最终打印 coc.nvim 配置：`icl-lsp` 的 stdio language server 配置，以及 `icl-format` 通过已确认的 coc 格式化适配插件所需配置／安装前提。若现有 coc 版本只支持 LSP 则提供 LSP 为主的配置，并明确 CLI 集成需要的适配器；不能编造 coc 原生配置键。同时给出 ICL filetype 检测和格式化调用，避免同时启用两个相互竞争的格式化提供者。不自动覆盖用户的 coc-settings.json。

## 设计确认后的执行

确认兼容策略与本设计后生成可执行计划，选择隔离工作区，按上述块顺序实施 TDD、回归和复核。逐块修正过程不需要再次逐项请求用户许可；只有新出现且会改变兼容边界的实质问题才需要补充澄清。

## Approved scope addendum (2026-10-22)

The user clarified that icl-lsp must also provide completion, definition and implementation navigation, and related language services. Shared AST-based declarations, references, scopes and source ranges will support these; no separate LSP parser. CodeGraph is now indexed at the original repository and is used for structural discovery. The documented evidence-backed extension policy was approved by Proceed.
