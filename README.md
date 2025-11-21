# Moonlight Godot

A Godot extension to use moonlight in Godot.

# Notes

从你提供的 `NvHTTP` 类的完整实现代码中，可以清晰地梳理出 Moonlight-Qt 与 NVIDIA GameStream（或兼容服务如 Sunshine）**服务端通信所依赖的应用层 API 接口列表**。这些接口均通过 HTTPS/HTTP GET 请求访问，路径为 `/api_name`，并携带查询参数。

以下是 **Moonlight 客户端调用的服务端 API 列表及其用途、参数和返回内容说明**：

---

### ✅ 1. `/serverinfo`
> **用途**：获取主机基本信息、状态、HTTPS 端口、配对状态、当前游戏等。  
> **调用时机**：连接前探测、状态轮询、配对前准备。  
> **请求方式**：GET  
> **关键参数**：
- `uniqueid=0123456789ABCDEF`（固定）
- `uuid=<随机UUID>`（每次不同）

> **返回 XML 示例**：
```xml
<root status_code="200">
  <hostname>MyPC</hostname>
  <HttpsPort>47984</HttpsPort>
  <state>SERVER_READY</state>
  <currentgame>0</currentgame>
  <GfeVersion>3.20.3.70</GfeVersion>
  <PairStatus>1</PairStatus> <!-- 1=已配对 -->
</root>
```

> **客户端用途**：
- 获取 `HttpsPort`（用于后续 HTTPS 请求）
- 检查 `PairStatus` 是否已配对
- 判断是否正在串流（`state` 是否含 `_SERVER_BUSY`）

---

### ✅ 2. `/launch`
> **用途**：启动指定应用并建立串流会话。  
> **调用时机**：用户点击“开始游戏”时。  
> **请求方式**：GET  
> **关键参数**（全部在 query string 中）：

| 参数 | 说明 |
|------|------|
| `appid` | 应用 ID（如 Steam 游戏 ID） |
| `mode` | 分辨率与帧率，格式 `WxHxFPS` |
| `sops` | 是否启用自动优化（0/1） |
| `rikey` | 远程输入 AES 密钥（hex） |
| `rikeyid` | AES IV 前 4 字节（大端整数） |
| `localAudioPlayMode` | 音频播放位置（1=客户端） |
| `surroundAudioInfo` | 音频通道配置 |
| `gcmap` / `remoteControllersBitmap` | 手柄映射位掩码 |
| `gcpersist` | 断开后是否保持手柄 |
| `hdrMode=1`（可选） | 启用 HDR |

> **返回 XML 关键字段**：
```xml
<root status_code="200">
  <sessionUrl0>rtsp://192.168.x.x:48010/xxx</sessionUrl0>
  <udpPort>48012</udpPort>
  <tcpPort>48014</tcpPort>
</root>
```

> **客户端用途**：
- 提取 `sessionUrl0` 用于 RTSP 控制通道
- 获取 UDP/TCP 端口用于视频/音频流传输

---

### ✅ 3. `/cancel`
> **用途**：终止当前串流会话（退出游戏）。  
> **调用时机**：用户点击“退出”或断开连接。  
> **请求方式**：GET  
> **参数**：仅 `uniqueid` 和 `uuid`  
> **返回**：成功返回 `<root status_code="200">`  
> **注意**：若非启动者调用，GFE 可能仍返回 200，需再查 `/serverinfo` 确认 `currentgame` 是否清零。

---

### ✅ 4. `/applist`
> **用途**：获取主机上可串流的应用列表。  
> **调用时机**：连接成功后加载游戏列表。  
> **请求方式**：GET  
> **返回 XML 示例**：
```xml
<root status_code="200">
  <App>
    <AppTitle>Milthm</AppTitle>
    <ID>705288528</ID>
    <IsHdrSupported>0</IsHdrSupported>
    <IsAppCollectorGame>0</IsAppCollectorGame>
  </App>
  <App>
    <AppTitle>桌面</AppTitle>
    <ID>1191261554</ID>
    ...
  </App>
</root>
```

> **客户端用途**：填充 UI 中的游戏/应用列表。

---

### ✅ 5. `/appasset`
> **用途**：获取应用的封面图（Box Art）。  
> **调用时机**：显示游戏列表时加载图标。  
> **请求方式**：GET  
> **参数**：
- `appid=12345`
- `AssetType=2`（表示封面图）
- `AssetIdx=0`（索引）

> **返回**：JPEG/PNG 图像二进制数据（非 XML）  
> **客户端用途**：显示游戏封面。

---

### ✅ 6. （隐式）SSL/TLS 双向认证（mTLS）
> 虽然不是独立 API，但所有 HTTPS 请求（`/serverinfo`, `/launch`, `/applist` 等）都要求：
- 客户端提供有效的 **客户端证书 + 私钥**（CN = `NVIDIA GameStream Client`）
- 服务端验证该证书（Sunshine/GFE 默认接受此 CN 的自签名证书）

> 在代码中通过：
```cpp
request.setSslConfiguration(IdentityManager::get()->getSslConfig());
```
设置客户端证书。

---

### 🔒 安全与认证机制总结

| 机制 | 说明 |
|------|------|
| **配对（Pairing）** | 通过 `/pair`（未在此类中实现，但在其他模块）完成，生成客户端证书 |
| **mTLS** | 所有 HTTPS 请求必须携带有效客户端证书 |
| **Server Cert Pinning** | 客户端保存服务端证书（`m_ServerCert`），用于 SSL 错误处理中比对 |
| **Unique ID** | 固定 `uniqueid=0123...` 允许多个 Moonlight 实例互相接管会话 |

---

### 📌 总结：Moonlight 调用的服务端 API 列表

| API 路径 | 方法 | 必需 | 主要用途 |
|--------|------|------|--------|
| `/serverinfo` | GET | ✅ | 获取主机状态、HTTPS 端口、配对状态 |
| `/launch` | GET | ✅ | 启动应用，获取 RTSP 会话 URL 和流端口 |
| `/cancel` | GET | ✅ | 退出当前应用 |
| `/applist` | GET | ✅ | 获取可串流应用列表 |
| `/appasset` | GET | ❌（可选） | 获取应用封面图 |
| （隐式）mTLS | — | ✅ | 所有 HTTPS 请求的身份认证 |

> ⚠️ 注意：`/pair`、`/unpair` 等配对接口虽未在此 `NvHTTP` 类中直接调用，但属于完整 GameStream 协议的一部分，通常在 `PairingManager` 或类似模块中实现。

---

如果你正在开发兼容服务（如自研 GameStream 服务器），**必须实现以上 5 个 API 并遵循其参数与响应格式**，才能被 Moonlight 正常使用。

从你提供的 `NvPairingManager` 类的实现代码中，可以完整解析出 **Moonlight 客户端与 GameStream 服务端进行配对（Pairing）过程中所调用的服务端 API 接口**。这些接口全部通过 HTTP GET 请求访问 `/pair` 和 `/unpair` 路径，并依赖多阶段挑战-响应协议完成安全配对。

---

## 🔐 配对流程概览

NVIDIA GameStream 的配对是一个 **5 阶段交互式认证过程**，目的是：
1. 安全交换客户端证书
2. 验证用户输入的 PIN 码
3. 防止中间人攻击（MITM）
4. 最终建立双向 TLS 信任（mTLS）

整个过程全部通过 `/pair` 接口的不同参数组合完成，并在失败时调用 `/unpair` 清理状态。

---

## ✅ 服务端 API 列表（配对专用）

### 1. **`/pair?phrase=getservercert&...`**
> **用途**：第一阶段 — 获取服务端证书  
> **请求参数**：
- `devicename=roth`（固定设备名）
- `updateState=1`
- `phrase=getservercert`
- `salt=<16字节随机盐的 hex>`
- `clientcert=<客户端 PEM 证书的 hex>`

> **返回 XML**：
```xml
<root status_code="200">
  <paired>1</paired>
  <plaincert>-----BEGIN CERTIFICATE-----...</plaincert>
</root>
```
> 若已存在配对会话，可能返回 `<paired>0</paired>` 或无 `plaincert`。

> **客户端动作**：
- 解析 `plaincert` 得到服务端证书
- 暂存该证书用于后续 HTTPS 请求（证书绑定）

---

### 2. **`/pair?clientchallenge=...`**
> **用途**：第二阶段 — 发送加密的客户端挑战  
> **请求参数**：
- `devicename=roth`
- `updateState=1`
- `clientchallenge=<AES-ECB(16字节随机数, AES密钥).toHex()>`

> 其中 **AES 密钥 = Hash(salt + PIN)**（SHA-1 或 SHA-256，取决于 GFE 版本）

> **返回 XML**：
```xml
<root status_code="200">
  <paired>1</paired>
  <challengeresponse><加密的响应数据 hex></challengeresponse>
</root>
```

> **客户端动作**：
- 用相同 AES 密钥解密 `challengeresponse`
- 提取服务端挑战、签名等数据用于下一阶段

---

### 3. **`/pair?serverchallengeresp=...`**
> **用途**：第三阶段 — 响应服务端挑战  
> **请求参数**：
- `devicename=roth`
- `updateState=1`
- `serverchallengeresp=<加密的哈希值 hex>`

> 加密内容为：`Hash(服务端挑战 + 客户端证书签名 + 客户端随机密钥)`（填充至 32 字节后 AES 加密）

> **返回 XML**：
```xml
<root status_code="200">
  <paired>1</paired>
  <pairingsecret><服务端密钥+签名 hex></pairingsecret>
</root>
```

> **客户端动作**：
- 解析 `pairingsecret` → 前 16 字节为 `serverSecret`，后为 `serverSignature`
- 验证 `serverSignature` 是否是对 `serverSecret` 的有效签名（使用服务端证书公钥）
  - 若失败 → **MITM 攻击，配对终止**
- 验证服务端是否知道正确 PIN（通过比对哈希）

---

### 4. **`/pair?clientpairingsecret=...`**
> **用途**：第四阶段 — 提交客户端配对密钥和签名  
> **请求参数**：
- `devicename=roth`
- `updateState=1`
- `clientpairingsecret=<clientSecret + sign(clientSecret) 的 hex>`

> **返回 XML**：
```xml
<root status_code="200">
  <paired>1</paired>
</root>
```

> **客户端动作**：确认服务端接受客户端身份

---

### 5. **`/pair?phrase=pairchallenge`**（通过 HTTPS）
> **用途**：第五阶段 — 最终验证（使用已建立的 mTLS）  
> **请求方式**：**HTTPS**（此时已信任服务端证书）  
> **请求参数**：
- `devicename=roth`
- `updateState=1`
- `phrase=pairchallenge`

> **返回 XML**：
```xml
<root status_code="200">
  <paired>1</paired>
</root>
```

> **意义**：确认整个配对链在安全通道下完成，服务端正式接受此客户端证书。

---

### ❌ 错误清理：`/unpair`
> **用途**：主动取消当前配对会话（如某阶段失败）  
> **调用时机**：任一阶段失败时  
> **请求参数**：无（仅 `devicename` 和 `uuid` 等通用参数）  
> **效果**：服务端清除正在进行的配对状态，允许重新开始

---

## 🔑 关键安全机制总结

| 机制 | 说明 |
|------|------|
| **PIN 绑定** | AES 密钥由 `salt + PIN` 哈希生成，错误 PIN 导致解密失败 |
| **证书交换** | 客户端上传证书，服务端返回其证书（`plaincert`） |
| **MITM 防护** | 服务端对其 `pairingsecret` 签名，客户端用服务端公钥验证 |
| **双向认证** | 配对成功后，客户端证书被服务端信任，后续所有 HTTPS 请求需携带该证书 |
| **版本适配** | Gen7+（GFE ≥7.x）使用 SHA-256，旧版用 SHA-1 |

---

## 📌 总结：配对过程调用的服务端 API

| 阶段 | API 路径 | 方法 | 协议 | 关键参数 | 目的 |
|------|--------|------|------|--------|------|
| 1 | `/pair` | GET | HTTP | `phrase=getservercert`, `salt`, `clientcert` | 获取服务端证书 |
| 2 | `/pair` | GET | HTTP | `clientchallenge` | 发送加密挑战 |
| 3 | `/pair` | GET | HTTP | `serverchallengeresp` | 响应服务端挑战 |
| 4 | `/pair` | GET | HTTP | `clientpairingsecret` | 提交客户端密钥与签名 |
| 5 | `/pair` | GET | **HTTPS** | `phrase=pairchallenge` | 最终安全确认 |
| - | `/unpair` | GET | HTTP | （无特殊参数） | 失败时清理配对状态 |

> ⚠️ 注意：所有 `/pair` 请求都包含 Moonlight 固定的 `uniqueid=0123456789ABCDEF` 和随机 `uuid`（见 `NvHTTP::openConnection` 实现）。

---

## 💡 对兼容服务（如 Sunshine）开发者的建议

若你要实现一个兼容 Moonlight 的 GameStream 服务端，**必须完整支持上述 5 阶段 `/pair` 协议**，包括：
- 正确解析 `clientcert` 并存储
- 生成符合格式的 `plaincert`
- 实现相同的 AES-ECB 加解密逻辑（**注意：ECB 不安全，但 GameStream 强制使用**）
- 对 `pairingsecret` 进行有效签名（使用服务端私钥）
- 支持 SHA-1 / SHA-256 双哈希模式（根据客户端上报的 `appVersion` 判断）

否则配对将失败，用户无法连接。

--- 

如需进一步分析每个阶段的数据结构或 OpenSSL 交互细节，也可以继续深入。