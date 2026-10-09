#pragma once
#include <wrl/client.h>

#include "D3d11ViewKeys.h"

#include "cave/core/base/NonCopyable.h"

namespace cave::render {

struct ColorAttachmentDesc;
struct DepthAttachmentDesc;

class D3d11ViewCache : public NonCopyable {
public:
    struct Stats {
        uint32_t rtv_count = 0;
        uint32_t dsv_count = 0;
        uint32_t rtv_hits = 0;
        uint32_t rtv_misses = 0;
        uint32_t dsv_hits = 0;
        uint32_t dsv_misses = 0;
    };

    explicit D3d11ViewCache(ID3D11Device* device) noexcept;
    ~D3d11ViewCache();

    void clear();

    ID3D11RenderTargetView* getOrCreateRtv(const ColorAttachmentDesc& desc);
    ID3D11DepthStencilView* getOrCreateDsv(const DepthAttachmentDesc& desc);

    Stats getStats() const;
    void resetStats() { m_stats = {}; }

private:
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> createRtv(const D3D11RtvKey& key);
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> createDsv(const D3D11DsvKey& key);

    ID3D11Device* m_device{ nullptr };

    HashMap<D3D11RtvKey, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_rtvs;
    HashMap<D3D11DsvKey, Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_dsvs;

    mutable Stats m_stats{};
};

}  // namespace cave::render
