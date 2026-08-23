#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "../Application.hpp"

namespace SGE::Samples {

class IScene {
public:
    virtual ~IScene() = default;
    virtual void setup(Application &app) { (void)app; }
    virtual void render(Application &app) = 0;
    virtual void drawUi(Application &app) { (void)app; }
    virtual const char *name() const = 0;
    virtual const char *group() const = 0;
};

struct SceneEntry {
    std::function<std::unique_ptr<IScene>()> factory;
    const char *group{nullptr};
    const char *name{nullptr};
};

class SceneRegistry {
public:
    static SceneRegistry &instance();

    void add(const char *group, const char *name,
        std::function<std::unique_ptr<IScene>()> factory);

    [[nodiscard]] const std::vector<SceneEntry> &entries() const { return m_entries; }

private:
    std::vector<SceneEntry> m_entries;
};

void registerBuiltinScenes();

}
