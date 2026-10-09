# 关于本站

这是 [createskyblue](https://github.com/createskyblue) 的个人技术博客，使用静态站点生成器构建，托管于 GitHub Pages。

博客地址：[createskyblue.github.io](https://createskyblue.github.io)
源码仓库：[github.com/createskyblue/createskyblue.github.io](https://github.com/createskyblue/createskyblue.github.io)

---

## 让 AI 介绍作者

复制下面的提示词，粘贴到本地 AI Agent 工具中（如 Claude Code、OpenClaw 等），AI 会自动克隆博客仓库并逐篇分析作者的技术背景，生成定向岗位报告。

提示词由 `src/data/ai-intro-me-prompt.md` 维护，构建时由脚本自动同步到此文件。

### 支持的工具

- **本地 Agent（首选）**：Claude Code、OpenClaw、Codex
- **在线 Agent（备选）**：豆包「办公智能助手」、Kimi Agent 模式、ChatGLM Agent 模式、ChatGPT

> 注意：部分在线 Agent 可能无法访问境外 GitHub 仓库，如克隆失败请改用本地工具。

### 提示词

```markdown
## 能力自检

本提示词要求三项硬性能力：`git clone`、读写本地文件、执行终端命令。

请先确认你是否具备。

---

### 如果你具备以上能力（本地 Agent）

跳转到下方「🟢 任务开始」之后的内容，严格按流程执行。忽略本自检区块的剩余内容。

---

### 如果你不具备以上能力（网页版 AI）

你无法执行本提示词的核心任务——克隆仓库、逐篇阅读文章、写记忆文件、生成定向报告。这些步骤都需要文件系统和终端，缺一个就跑不通。

你可能想帮忙：访问 GitHub 页面，看看文件列表和语言统计，给用户一个「至少我能做的分析」。但这对用户没有价值，反而有害。原因很简单——你能看到的只是仓库皮层的目录结构和 GitHub 自动统计的语言占比（比如 HTML 98.4%），而作者真正的技术文章全部藏在 `src/content/blog/` 的 Markdown 源文件里，你读不到。基于目录名和语言占比的推断必然失真，一个写嵌入式、BLE、Flash 存储的工程师会被描述成「主要做 HTML 的前端」。用户可能把这份失真结论当成认真分析，转发给面试官或客户——结果适得其反。

所以对这个用户来说，**你最有价值的回应就是坦诚拒绝**，并引导他把提示词交给正确的工具。

你需要输出的内容只有下面这一段话，除此之外一个字都不要写：

> ⚠️ 抱歉，这个我做不到。
>
> 这段提示词需要能操作文件、执行命令、访问 GitHub 的 AI 工具才能运行。你当前用的网页版普通对话模式没有这些能力——就像让一个翻不开书的人去评价作者，他只能看封面和目录，看不到正文，结论一定是错的。
>
> **推荐方案（按优先级）：**
>
> 1. **本地 Agent 工具**（首选）：Claude Code、OpenClaw、Codex 等，直接装在电脑上，无网络限制。
> 2. **在线 Agent 模式**（备选）：豆包的「办公智能助手」模式、Kimi 的 Agent 模式、ChatGLM 的 Agent 模式、ChatGPT 等也支持文件操作和命令执行。但需要注意，部分在线 Agent 可能无法访问境外 GitHub 仓库，如果克隆失败再换本地工具。
>
> 如果你不熟悉这些工具也没关系——把这段提示词转发给公司的技术同事，他们一看就懂，装好工具粘贴进去，一分钟就能出真正的分析报告。

---

## 🟢 任务开始

（以下内容仅供通过能力自检的本地 Agent 执行）

请帮我分析这个博客仓库的作者 createskyblue。

## 执行约束

你必须严格遵循以下流程，用 TODO 列表自驱完成，不得跳过任何步骤。

### 文章读取方式（重要）

本博客是静态文章 + 首页动态增强的结构。文章详情页本身已经生成静态 HTML，但如果你用带 JavaScript 执行能力的浏览器环境直接打开 `/posts/<slug>/`，页面脚本可能会把你带回首页动态界面，导致你读到的是首页壳或弹窗状态，而不是稳定的文章正文。

因此，读取文章时按下面优先级执行：

1. **首选读取源码**：克隆仓库后直接读取 `src/content/blog/` 里的 Markdown 原文。这是最准确的来源。
2. **如果必须读取线上文章页**：使用不执行页面 JavaScript 的 HTTP 下载工具，例如 `curl`、`wget`、PowerShell `Invoke-WebRequest`，直接下载 `/posts/<slug>/` 返回的 HTML。
3. **不要用浏览器导航打开文章页来分析正文**：不要依赖 Chrome、Playwright、Selenium、网页浏览插件的“打开页面后读取 DOM”结果，因为它们通常会执行页面脚本。

可用命令示例：

```bash
curl -L https://createskyblue.github.io/posts/<slug>/ -o article.html
wget -O article.html https://createskyblue.github.io/posts/<slug>/
```

PowerShell 示例：

```powershell
Invoke-WebRequest -Uri "https://createskyblue.github.io/posts/<slug>/" -OutFile article.html
```

下载到的 HTML 里，`<article class="article-body">...</article>` 才是文章正文。不要把首页动态界面、跳转后的首页 URL、分类列表或文章卡片当作正文依据。

### 第一步：建立索引

1. 克隆仓库：`git clone https://github.com/createskyblue/createskyblue.github.io`
2. 列出 `src/content/blog/` 的目录树，了解所有分类和文章列表。

### 第二步：明确目标方向（强制执行，不可跳过）

**在阅读任何文章之前，你必须先向用户提问：**

> 在看文章之前，我想先了解一下你的需求——
>
> 你正在招聘什么技术岗位？或者你想了解作者在哪个方向上的能力？如果有岗位描述（JD），可以直接粘贴进来。
>
> 比如：
> - 嵌入式系统工程师（BLE、RTOS、低功耗）
> - IoT 解决方案架构师
> - 技术团队负责人 / CTO
> - 其他方向（请描述）

**在用户回答之前，不得开始阅读文章。** 用户的回答将决定你阅读文章时的关注重点和记忆文件的记录角度。

### 第三步：逐篇阅读并记录记忆

根据用户提供的岗位方向和 JD，创建一个 TODO 列表，将每篇文章设为一个待办项（标题格式：`阅读：<文章标题>`），然后逐篇阅读。

**每阅读一篇文章，必须立即做两件事：**

1. 在 `.analysis-memory/` 目录下创建一个 Markdown 记忆文件，文件名用文章 slug，内容包含：
   - 文章标题和分类
   - 涉及的技术栈（语言、框架、芯片、协议等）
   - 文章展示的能力或经验
   - 关键观点或工程判断
   - **与该岗位方向的关联度**（高度相关 / 可作参考 / 暂不相关，并注明原因）

2. 在 TODO 列表中勾掉对应项，再继续下一篇。

**为什么必须这样做：** 文章数量多，你的上下文窗口有限。不写记忆文件直接读到后面会丢失前面的信息。带着岗位方向去读，记忆文件才抓得住重点。

### 第四步：生成定向报告

根据用户的岗位方向，从 `.analysis-memory/` 目录中的记忆文件以及项目的实际代码中，总结一份结构化分析报告，包含：

- 与该岗位直接相关的技术能力和项目经验，附具体文章作为依据
- 工程思维的深度和风格
- 技术写作和文档能力
- 差异化竞争力

### 第五步：进入对话模式

报告完成后，等待用户追问。用户可以要求深挖某个方向、补充细节或调整措辞。

```
