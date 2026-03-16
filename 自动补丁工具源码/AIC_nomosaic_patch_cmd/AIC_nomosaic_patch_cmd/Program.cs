using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using dnlib.DotNet;
using dnlib.DotNet.Emit;


namespace AIC_nomosaic_patch_cmd
{
    class Program
    {
        static void Main(string[] args)
        {
            string dllPath = null;
            string outPath = null;
            bool? targetValue = null;

            // -----------------------
            // 参数解析
            // -----------------------
            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i].Trim().ToLower();

                // -m=false / -mode=false 或 -m false
                if (arg == "-m" || arg == "-mode")
                {
                    if (i + 1 < args.Length)
                    {
                        string val = args[i + 1].Trim().ToLower();
                        if (val == "true") targetValue = true;
                        else if (val == "false") targetValue = false;
                        else
                        {
                            Console.WriteLine("请指定 -m/--mode=true 或 false");
                            return;
                        }
                        i++;
                    }
                    else
                    {
                        Console.WriteLine("请指定 -m/--mode=true 或 false");
                        return;
                    }
                }
                else if (arg.StartsWith("-m=") || arg.StartsWith("-mode="))
                {
                    string val = arg.Split('=')[1].Trim().ToLower();
                    if (val == "true") targetValue = true;
                    else if (val == "false") targetValue = false;
                    else
                    {
                        Console.WriteLine("请指定 -m/--mode=true 或 false");
                        return;
                    }
                }
                else if (arg.StartsWith("-dll="))
                {
                    dllPath = arg.Substring(5).Trim();
                }
                else if (arg == "-dll")
                {
                    if (i + 1 < args.Length) dllPath = args[i + 1].Trim();
                    i++;
                }
                else if (arg.StartsWith("-out="))
                {
                    outPath = arg.Substring(5).Trim();
                }
                else if (arg == "-out")
                {
                    if (i + 1 < args.Length && i + 1 < args.Length) outPath = args[i + 1].Trim();
                    i++;
                }
            }

            if (dllPath == null)
            {
                Console.WriteLine("请指定 -dll=<dll路径>");
                return;
            }

            if (targetValue == null)
            {
                Console.WriteLine("请指定 -m/--mode=true 或 false");
                return;
            }

            if (string.IsNullOrEmpty(outPath))
                outPath = dllPath; // 默认覆盖原 DLL

            try
            {
                // -----------------------
                // 加载 DLL（无依赖模式）
                // -----------------------
                var module = ModuleDefMD.Load(dllPath);

                // -----------------------
                // 找到 MosaicShower.FnDrawMosaic
                // -----------------------
                var type = module.Types.FirstOrDefault(t => t.Name == "MosaicShower");
                if (type == null)
                {
                    Console.WriteLine("notfound");
                    return;
                }

                var method = type.Methods.FirstOrDefault(m => m.Name == "FnDrawMosaic");
                if (method == null || !method.HasBody)
                {
                    Console.WriteLine("notfound");
                    return;
                }

                // -----------------------
                // 修改方法 IL（只改 true/false）
                // -----------------------
                bool already = true;
                foreach (var instr in method.Body.Instructions)
                {
                    if ((instr.OpCode == OpCodes.Ldc_I4_1 && targetValue == false) ||
                        (instr.OpCode == OpCodes.Ldc_I4_0 && targetValue == true))
                    {
                        instr.OpCode = targetValue.Value ? OpCodes.Ldc_I4_1 : OpCodes.Ldc_I4_0;
                        already = false;
                    }
                    else if ((instr.OpCode == OpCodes.Ldc_I4_1 && targetValue == true) ||
                             (instr.OpCode == OpCodes.Ldc_I4_0 && targetValue == false))
                    {
                        // 已经是目标值，不改
                        continue;
                    }
                }

                if (already)
                {
                    Console.WriteLine("repeat");
                    return;
                }

                // -----------------------
                // 保存 DLL
                // -----------------------
                module.Write(outPath);
                Console.WriteLine("TRUE");
            }
            catch (Exception ex)
            {
                Console.WriteLine("修改失败: " + ex.Message);
            }
        }
    }
}