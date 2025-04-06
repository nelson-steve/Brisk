#pragma once

#include "RenderPass.hpp"

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <queue>

namespace Brisk
{
    struct VulkanResource;
    struct DX12Resource;

    struct ResourceHandle {
        uint32_t id;
        ResourceHandle() : id(0) {}
        explicit ResourceHandle(uint32_t _id) : id(_id) {}
        bool operator==(const ResourceHandle& other) const { return id == other.id; }
    };

    class RenderGraph {
    public:
        struct PassNode {
            std::string name;
            std::vector<ResourceHandle> inputs;
            std::vector<ResourceHandle> outputs;
            int dependencyCount = 0; // Tracks unresolved dependencies
            std::vector<PassNode*> dependents; // Passes that depend on this one
        };

        void AddPass(const std::string& name,
            const std::vector<ResourceHandle>& inputs,
            const std::vector<ResourceHandle>& outputs) {
            PassNode node{ name, inputs, outputs, 0, {} };
            passMap[name] = &passes.emplace_back(std::move(node));
        }

        void BuildExecutionOrder() {
            std::unordered_map<ResourceHandle, PassNode*> resourceProducers;
            for (auto& pass : passes) {
                for (const auto& output : pass.outputs) {
                    resourceProducers[output] = &pass;
                }
            }
            for (auto& pass : passes) {
                for (const auto& input : pass.inputs) {
                    if (resourceProducers.count(input)) {
                        PassNode* producer = resourceProducers[input];
                        producer->dependents.push_back(&pass);
                        pass.dependencyCount++;
                    }
                }
            }
        }

        void Execute() {
            std::queue<PassNode*> readyPasses;
            for (auto& pass : passes) {
                if (pass.dependencyCount == 0) {
                    readyPasses.push(&pass);
                }
            }
            while (!readyPasses.empty()) {
                PassNode* node = readyPasses.front();
                readyPasses.pop();
                node->pass->Execute();
                for (PassNode* dependent : node->dependents) {
                    if (--dependent->dependencyCount == 0) {
                        readyPasses.push(dependent);
                    }
                }
            }
        }

    private:
        std::vector<PassNode> passes;
        std::unordered_map<std::string, PassNode*> passMap;
    };

    class RenderGraphBuilder {
    public:
        RenderGraphBuilder(RenderGraph& graph) : graph(graph) {}

        RenderGraphBuilder& AddPass(const std::string& name,
            const std::vector<ResourceHandle>& inputs = {},
            const std::vector<ResourceHandle>& outputs = {}) {
            graph.AddPass(name, inputs, outputs);
            return *this;
        }

        void Build() {
            graph.BuildExecutionOrder();
        }

    private:
        RenderGraph& graph;
    };
}