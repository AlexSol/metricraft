#include "scheduler.hpp"
#include "exec_runner.hpp"
// #include "http.hpp"
#include "manifest.hpp"
#include <filesystem>
#include <thread>
#include <iostream>

int main(){
    Scheduler sch;
    // Http http;

    // MVP: завантажимо всі manifests/*.yaml
    for (auto& p : std::filesystem::directory_iterator("/home/alex/fun_projects/metricraft/plugins"))
    {
        if (p.path().extension() == ".yaml"){
            std::clog << "Found plugin manifest: " << p.path() << std::endl;

            auto m = Manifest::load(p.path().string());
            if (m.runtime=="exec") {
                std::clog << "Plugin runtime: exec" << std::endl;
                sch.add(Task{m, std::make_unique<ExecRunner>(m)});
            } else if (m.runtime=="python") {
                std::clog << "Plugin runtime: python" << std::endl;
                // sch.add(Task{m, std::make_unique<PyRunner>(m, pyenv, "plugins")});
            } // wasm/lua далі

        }
    }

    // std::jthread http_thr([&]{ http.serve(9100); });
    sch.run([&](const std::string& name, const MetricsBatch& b){
        // http.put(name,b);

        std::clog << "Metrics for '" << name << "':\n";
    });
    
    return 0;
}
