#pragma once

#include <windows.h>
#include <iostream>
#include "define.h"

bool IsDotNetInstalled() {
    HKEY hKey;
    DWORD dwType = 0;
    DWORD dwData = 0;
    DWORD dwSize = sizeof(DWORD);

    // 查询 .NET Framework 4.5 及以上
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, L"Release", NULL, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            // dwData >= 378389 表示安装了 .NET Framework 4.5+
            return dwData >= 378389;
        }
        RegCloseKey(hKey);
    }
    return false;
}

#include <fstream>

bool ExtractResourceToFile(int resourceID, const wchar_t* type, const std::wstring& outputPath) {
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCE(resourceID), type);
    if (!hRes) return false;

    HGLOBAL hData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    void* data = LockResource(hData);

    std::ofstream file(outputPath, std::ios::binary);
    file.write((char*)data, size);
    file.close();

    return true;
}
#include <string>

bool RunCommandHidden(const std::wstring& cmd)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // 隐藏窗口

    // CreateProcess 需要命令行可写
    wchar_t* cmdLine = new wchar_t[cmd.size() + 1];
    wcscpy_s(cmdLine, cmd.size() + 1, cmd.c_str());

    bool result = CreateProcessW(
        NULL,      // 可执行文件名为空，命令行里指定
        cmdLine,   // 命令行
        NULL, NULL,// 默认安全属性
        FALSE,     // 不继承句柄
        0,         // 默认创建标志
        NULL, NULL,// 默认环境和目录
        &si, &pi
    );

    if (!result) {
        delete[] cmdLine;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    delete[] cmdLine;
    return true;
}
#include <vector>
#include <string>
#include <regex>
bool patch_il(const std::wstring& input_il, const std::wstring& output_il)
{
    std::wifstream vanilla(input_il);
    std::wofstream mod(output_il);

    if (!vanilla.is_open()) {
        MessageBox(hWnd, L"严重错误，反编译文件丢失", L"错误", MB_OK | MB_ICONHAND);
        //std::wcout << L"Error: " << input_il << L" not found\n";
        return false;
    }

    std::vector<std::wstring> buffer;
    bool patched = false;

    std::wregex func_end(L"// end of method MosaicShower::FnDrawMosaic");
    std::wregex feature_code(L"ldc\\.i4\\.1");

    std::wstring line;

    while (std::getline(vanilla, line)) {
        line += L"\n";
        buffer.push_back(line);

        if (std::regex_search(line, func_end)) {

            for (size_t i = 0; i < buffer.size(); i++) {

                if (std::regex_search(buffer[i], feature_code)) {
                    //MessageBox(NULL, L"Info: Discovery feature:", L"info", MB_OK | MB_ICONWARNING);
                    //std::wcout << L"Info: Discovery feature: " << buffer[i];

                    buffer[i] = std::regex_replace(
                        buffer[i],
                        feature_code,
                        L"ldc.i4.0"
                    );

                    patched = true;
                }
            }
        }

        if (buffer.size() > 5) {
            mod << buffer[0];
            buffer.erase(buffer.begin());
        }
    }

    for (auto& l : buffer)
        mod << l;

    if (patched){
        //MessageBox(NULL, L"Info: Patched Success!", L"info", MB_OK | MB_ICONWARNING);
    }else {
        //MessageBox(NULL, L"Error: Patched Failed.", L"info", MB_OK | MB_ICONWARNING);
        //std::wcout << L"Error: Patched Failed.\n";
        MessageBox(hWnd, L"补丁已安装！\n若游戏画面仍无补丁，请重装游戏再安装补丁。", L"错误", MB_OK | MB_ICONHAND);
        return false;
    }

    return true;
}
bool unpatch_il(const std::wstring& input_il, const std::wstring& output_il)
{
    std::wifstream vanilla(input_il);
    std::wofstream mod(output_il);

    if (!vanilla.is_open()) {
        MessageBox(hWnd, L"严重错误，反编译文件丢失", L"错误", MB_OK | MB_ICONHAND);
        //std::wcout << L"Error: " << input_il << L" not found\n";
        return false;
    }

    std::vector<std::wstring> buffer;
    bool patched = false;

    std::wregex func_end(L"// end of method MosaicShower::FnDrawMosaic");
    std::wregex feature_code(L"ldc\\.i4\\.0");

    std::wstring line;

    while (std::getline(vanilla, line)) {
        line += L"\n";
        buffer.push_back(line);

        if (std::regex_search(line, func_end)) {

            for (size_t i = 0; i < buffer.size(); i++) {

                if (std::regex_search(buffer[i], feature_code)) {
                    //MessageBox(NULL, L"Info: Discovery feature:", L"info", MB_OK | MB_ICONWARNING);
                    //std::wcout << L"Info: Discovery feature: " << buffer[i];

                    buffer[i] = std::regex_replace(
                        buffer[i],
                        feature_code,
                        L"ldc.i4.1"
                    );

                    patched = true;
                }
            }
        }

        if (buffer.size() > 5) {
            mod << buffer[0];
            buffer.erase(buffer.begin());
        }
    }

    for (auto& l : buffer)
        mod << l;

    if (patched) {
        //MessageBox(NULL, L"Info: Unpatched Success!", L"info", MB_OK | MB_ICONWARNING);
    }
    else {
        //MessageBox(NULL, L"Error: Unpatched Failed.", L"info", MB_OK | MB_ICONWARNING);
        //std::wcout << L"Error: Unpatched Failed.\n";
        MessageBox(hWnd, L"未找到可卸载的补丁！", L"错误", MB_OK | MB_ICONHAND);
        return false;
    }

    return true;
}
bool IsFileInUse(const std::wstring& filePath)
{
    // 尝试以独占方式打开文件
    HANDLE hFile = CreateFileW(
        filePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,  // 尝试读写
        0,                             // 不共享
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION)
        {
            return true; // 文件被占用
        }
        return false; // 文件不存在或其他错误
    }

    // 文件可以打开，说明没被占用
    CloseHandle(hFile);
    return false;
}
bool patch(const std::wstring& ildasmPath,
    const std::wstring& ilasmPath,
    const std::wstring& dllPath,bool aaa)
{
    if (IsFileInUse(AssemblyDllPath)) {
        MessageBox(hWnd, L"游戏文件被占用，请先解除占用。", L"提示", MB_OK);
        return false;
    }
        
    std::wstring dumpIL = dllPath + L".il";
    std::wstring dumpModIL = dllPath + L".mod.il";
    std::wstring outputDLL = dllPath;

    std::wstring cmd =
        L"\"" + ildasmPath + L"\" \"" + dllPath +
        L"\" /OUT=\"" + dumpIL + L"\" /TEXT /UTF8";
    //MessageBox(NULL, cmd.c_str(), L"info", MB_OK | MB_ICONWARNING);
    HWND hButton;
    if (aaa == false) hButton = GetDlgItem(hWnd, IDM_test); // 获取按钮句柄
    else hButton = GetDlgItem(hWnd, IDM_test2); // 获取按钮句柄
    SetWindowTextW(hButton, L"正在反编译...");
    if (!RunCommandHidden(cmd)) {
        MessageBox(hWnd, L"DLL反编译失败", L"错误", MB_OK | MB_ICONHAND);
        return false;
    }
    if (aaa == false) {
        SetWindowTextW(hButton, L"正在安装...");
        if (patch_il(dumpIL, dumpModIL)) {

        }
        else return false;
    }
    else {
        SetWindowTextW(hButton, L"正在卸载...");
        if (unpatch_il(dumpIL, dumpModIL)) {

        }
        else return false;
    }

    std::wstring ilasmCmd =
        L"\"" + ilasmPath + L"\" \"" + dumpModIL +
        L"\" /DLL /OUTPUT=\"" + outputDLL + L"\"";
    SetWindowTextW(hButton, L"正在编译...");
    if (!RunCommandHidden(ilasmCmd)) {
        MessageBox(hWnd, L"DLL编译失败", L"错误", MB_OK | MB_ICONHAND);
        return false;
    }
    return true;
}

#include <commdlg.h>
std::wstring OpenExeFileDialog()
{
    OPENFILENAMEW ofn = {};
    wchar_t szFile[MAX_PATH] = L"AliceInCradle.exe";

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"可执行文件 (*.exe)\0*.exe\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = L"请选择游戏主程序";
    if (GetOpenFileNameW(&ofn))
    {
        // 用户选择了文件，返回路径
        return std::wstring(szFile);
    }
    else
    {
        // 用户取消或出错，返回空字符串
        return L"";
    }
}
std::wstring GetExeFolder(const std::wstring& exePath)
{
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        return exePath.substr(0, pos); // 返回目录
    else
        return L""; // 没有父目录
}
bool CheckAssemblyDll(const std::wstring& baseDir)
{
    // 构造 DLL 路径
    std::wstring dllPath = baseDir + L"\\AliceInCradle_Data\\Managed\\Assembly-CSharp.dll";

    DWORD attrs = GetFileAttributesW(dllPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES)
        return false; // 文件不存在或路径错误

    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
        return false; // 是目录，不是文件

    return true; // 文件存在
}
std::wstring GetVer(const std::wstring& folderPath)
{
    // 找到最后一个斜杠，取最后的文件夹名
    size_t lastSlash = folderPath.find_last_of(L"\\/");
    std::wstring folderName;
    if (lastSlash != std::wstring::npos)
        folderName = folderPath.substr(lastSlash + 1);
    else
        folderName = folderPath; // 没有斜杠，整个字符串就是文件夹名

    // 统计下划线数量
    size_t firstUnderscore = folderName.find(L'_');
    if (firstUnderscore == std::wstring::npos)
        return L"未知"; // 没有下划线

    size_t lastUnderscore = folderName.rfind(L'_');
    if (firstUnderscore != lastUnderscore)
        return L"未知"; // 多个下划线

    if (firstUnderscore + 1 >= folderName.size())
        return L"未知"; // 下划线在末尾

    // 返回下划线后的部分
    return folderName.substr(firstUnderscore + 1);
}

std::wstring GetCurrentExeFolder()
{
    wchar_t buffer[MAX_PATH] = { 0 };
    DWORD len = GetModuleFileNameW(NULL, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
    {
        return L""; // 获取失败
    }

    std::wstring path(buffer);
    // 找最后一个斜杠
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        return path.substr(0, pos); // 返回目录
    else
        return L""; // 不存在斜杠，返回空
}

bool IsAlreadyRunning()
{
    // 创建一个全局互斥体
    HANDLE hMutex = CreateMutexW(NULL, FALSE, L"MyUniqueAppMutexName_12345");
    if (hMutex == NULL)
    {
        // 创建失败，认为程序不可运行
        return true;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // 互斥体已存在 → 已经有实例运行
        CloseHandle(hMutex);
        return true;
    }

    // 互斥体创建成功，当前是唯一实例
    return false;
}