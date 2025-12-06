using Godot;
using System;
using System.Net.Http;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

/// <summary>
/// 简单的 mTLS HTTP 客户端工具类：
/// 给定客户端证书 + 私钥（PEM）、请求方法、URL、可选请求体、是否验证服务器证书，返回响应内容。
/// </summary>
[GlobalClass]
public partial class MtlsHttpClient : Node
{
    /// <summary>
    /// 发送一个 mTLS HTTP 请求。
    /// </summary>
    /// <param name="method">HTTP 方法，例如 "GET"、"POST"、"PUT"、"DELETE"。</param>
    /// <param name="url">请求 URL。</param>
    /// <param name="clientCertPem">客户端证书 PEM 字符串（包含 -----BEGIN CERTIFICATE----- ...）。</param>
    /// <param name="clientKeyPem">客户端私钥 PEM 字符串（包含 -----BEGIN PRIVATE KEY----- 或 -----BEGIN RSA PRIVATE KEY----- ...）。</param>
    /// <param name="body">可选请求体内容（字符串），例如 JSON。GET 请求通常传 null。</param>
    /// <param name="validateServerCertificate">是否验证服务器证书（默认 true）。设为 false 将接受任何服务器证书（不安全，仅调试用）。</param>
    /// <param name="contentType">请求体 Content-Type，默认 "application/json"。</param>
    /// <param name="cancellationToken">取消 token，可选。</param>
    /// <returns>响应内容字符串。</returns>
    public async Task<string> SendRequestAsync(
        string method,
        string url,
        string clientCertPem,
        string clientKeyPem,
        string? body = null,
        bool validateServerCertificate = true,
        string contentType = "application/json",
        CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrWhiteSpace(method))
            throw new ArgumentException("HTTP method cannot be empty.", nameof(method));

        if (string.IsNullOrWhiteSpace(url))
            throw new ArgumentException("URL cannot be empty.", nameof(url));

        if (string.IsNullOrWhiteSpace(clientCertPem))
            throw new ArgumentException("Client certificate PEM cannot be empty.", nameof(clientCertPem));

        if (string.IsNullOrWhiteSpace(clientKeyPem))
            throw new ArgumentException("Client private key PEM cannot be empty.", nameof(clientKeyPem));

        // 1. 从 PEM 证书 + 私钥构建 X509Certificate2（带私钥）
        // 需要 .NET 5+ / 6+ 支持 CreateFromPem
        X509Certificate2 clientCertificate = X509Certificate2.CreateFromPem(clientCertPem, clientKeyPem);

        // 某些平台上临时证书对象的私钥不可导出 / 不可用于 TLS，
        // 通过 Export + Import 一次可以得到一个更“正常”的 X509Certificate2 实例。
        clientCertificate = new X509Certificate2(
            clientCertificate.Export(X509ContentType.Pkcs12));

        // 2. 配置 HttpClientHandler
        var handler = new HttpClientHandler
        {
            // 将客户端证书添加到握手中
            ClientCertificateOptions = ClientCertificateOption.Manual
        };
        handler.ClientCertificates.Add(clientCertificate);

        // 是否验证服务器证书
        if (!validateServerCertificate)
        {
            // 接受任何服务器证书（不安全，仅调试用）
            handler.ServerCertificateCustomValidationCallback = (
                HttpRequestMessage _,
                X509Certificate2? _,
                X509Chain? _,
                SslPolicyErrors _) => true;
        }
        // 否则保持默认验证逻辑（使用系统证书链）

        // 3. 使用 HttpClient 发送请求
        using var httpClient = new System.Net.Http.HttpClient(handler);

        var request = new HttpRequestMessage(new HttpMethod(method), url);

        if (!string.IsNullOrEmpty(body))
        {
            request.Content = new StringContent(body, Encoding.UTF8, contentType);
        }

        using HttpResponseMessage response = await httpClient.SendAsync(request, cancellationToken);

        // 如需手动处理非 2xx，可去掉 EnsureSuccessStatusCode，改成根据 StatusCode 分支。
        response.EnsureSuccessStatusCode();

        string responseText = await response.Content.ReadAsStringAsync(cancellationToken);
        return responseText;
    }

    /// <summary>
    /// 同步封装（内部等待异步）。在 Godot 场景中推荐仍使用异步版本。
    /// </summary>
    public string SendRequest(
        string method,
        string url,
        string clientCertPem,
        string clientKeyPem,
        string? body = null,
        bool validateServerCertificate = true,
        string contentType = "application/json")
    {
        return SendRequestAsync(
                method,
                url,
                clientCertPem,
                clientKeyPem,
                body,
                validateServerCertificate,
                contentType)
            .GetAwaiter()
            .GetResult();
    }
}
