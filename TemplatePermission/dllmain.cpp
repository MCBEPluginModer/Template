// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "PAPI.h"
#pragma comment(lib,"Permissions Ex API.lib")

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        LL::registerPlugin("Template", "Шаблон с интеграцией прав из Permissions Ex", LL::Version(1, 0, 2, LL::Version::Release));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


void entry();

extern "C" {
    _declspec(dllexport) void onPostInit() {
        std::ios::sync_with_stdio(false);
        entry();
    }
}

#include <MC/Command.hpp>
#include <MC/CommandRegistry.hpp>
#include <MC/CommandOrigin.hpp>
#include <MC/CommandOutput.hpp>
#include <MC/Dimension.hpp>
#include <MC/Player.hpp>
#include <RegCommandAPI.h>
#include <third-party/yaml-cpp/yaml.h>
#include <vector>

class TestCmd : public Command
{
    string arg;
public:
    void execute(CommandOrigin const& ori, CommandOutput& outp) const override
    {
        auto id = ori.getDimension()->getDimensionId(); //получаем id измерения игрока
        string dim; 
        if (id == 0)
            dim = "OverWorld";
        else if (id == 1)
            dim = "Nether";
        else if (id == 2)
            dim = "End";
        string nick = ((Player*)ori.getPlayer())->getName(); //получаем ник
        Users users;
        YAML::Node node = YAML::LoadFile("plugins/Permissions Ex/users.yml"); //доступ к бд игроков
        for (const auto& p : node["users"])
            users.users.push_back(p.as<_User>());
        auto nick1 = split(nick, " ");
        string res_nick;
        for (auto n : nick1) //ищем его реальный ник
        {
            for (auto v : users.users)
            {
                if (n == v.nickname)
                {
                    res_nick = n;
                    break;
                }
            }
        }
        string perm = "testcmd.run"; //право
        string error_msg = get_msg("permissionDenied"); //сообщение в случае отсуствия права
        string error_msg1 = get_msg("invalidArgument"); //сообщение в случае оишбки аргумента
        if (arg == "") //в случае пустого значения аргумента сообщаем об ошибке аргумента
        {
            outp.error(error_msg1);
            return;
        }
       if (((checkPerm(res_nick, perm) || checkPerm(res_nick, "plugins.*") || checkPermWorlds(res_nick, perm, dim) || checkPermWorlds(res_nick, "plugins.*", dim)))) //проверка прав,plugins.* - все права
       {
           outp.success("Succesfull!");
           return;
       }
       else //нету права - выводим сообщение об ошибке
       {
           outp.error(error_msg);
           return;
       }
    }
    static void setup(CommandRegistry* r)
    {
        r->registerCommand(
            "test", "Test cmd", CommandPermissionLevel::Any, { (CommandFlagValue)0 },
            { (CommandFlagValue)0x80 });
        r->registerOverload<TestCmd>("test", RegisterCommandHelper::makeMandatory(&TestCmd::arg, "arg"));
    }
};

#include <EventAPI.h>

void entry()
{
    Event::RegCmdEvent::subscribe([](const Event::RegCmdEvent& ev) 
    {
            TestCmd::setup(ev.mCommandRegistry); //регистрируем команду
            return 1;
    });
}