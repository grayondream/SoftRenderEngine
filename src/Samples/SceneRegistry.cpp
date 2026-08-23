#include "IScene.hpp"

namespace SGE::Samples {

SceneRegistry &SceneRegistry::instance(){
    static SceneRegistry reg;
    return reg;
}

void SceneRegistry::add(const char *group, const char *name,
    std::function<std::unique_ptr<IScene>()> factory){
    m_entries.push_back(SceneEntry{std::move(factory), group, name});
}

}
