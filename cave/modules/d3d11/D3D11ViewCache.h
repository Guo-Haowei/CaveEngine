#pragma once
#include <wrl/client.h>

#include "D3D11ViewKeys.h"

#include "cave/core/base/NonCopyable.h"

namespace cave::render {

struct ColorAttachmentDesc;
struct DepthAttachmentDesc;

class D3D11ViewCache : public NonCopyable {
public:
    struct Stats {
        uint32_t rtv_count = 0;
        uint32_t dsv_count = 0;
        uint32_t rtv_hits = 0;
        uint32_t rtv_misses = 0;
        uint32_t dsv_hits = 0;
        uint32_t dsv_misses = 0;
    };

    explicit D3D11ViewCache(ID3D11Device* p_device) noexcept;
    ~D3D11ViewCache();

    void Clear();

    ID3D11RenderTargetView* GetOrCreateRtv(const ColorAttachmentDesc& p_desc);
    ID3D11DepthStencilView* GetOrCreateDsv(const DepthAttachmentDesc& p_desc);

    Stats GetStats() const;
    void ResetStats() { m_stats = {}; }

private:
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CreateRtv(const D3D11RtvKey& p_key);
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> CreateDsv(const D3D11DsvKey& p_key);

    ID3D11Device* m_device{ nullptr };

    std::unordered_map<D3D11RtvKey, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_rtvs;
    std::unordered_map<D3D11DsvKey, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_dsvs;

    mutable Stats m_stats{};
};

}  // namespace cave::render
