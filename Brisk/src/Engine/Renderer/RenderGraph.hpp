#pragma once
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

struct VulkanResource;
struct DX12Resource;

struct ResourceHandle {
    uint32_t id;
    ResourceHandle() : id(0) {}
    explicit ResourceHandle(uint32_t _id) : id(_id) {}
    bool operator==(const ResourceHandle& other) const { return id == other.id; }
};

class RenderPass {
public:
    virtual void Execute() = 0;
    virtual ~RenderPass() = default;
};

class RenderGraph {
public:
    struct PassNode {
        std::string name;
        std::unique_ptr<RenderPass> pass;
        std::vector<ResourceHandle> inputs;
        std::vector<ResourceHandle> outputs;
    };

    void AddPass(const std::string& name, std::unique_ptr<RenderPass> pass,
        const std::vector<ResourceHandle>& inputs,
        const std::vector<ResourceHandle>& outputs) {
        PassNode node{ name, std::move(pass), inputs, outputs };
        passes.push_back(std::move(node));
    }

    void Execute() {
        for (auto& node : passes) {
            node.pass->Execute();
        }
    }

private:
    std::vector<PassNode> passes;
};

class VulkanRenderPass : public RenderPass {
public:
    void Execute() override {
    }
};

class DX12RenderPass : public RenderPass {
public:
    void Execute() override {
    }
};
