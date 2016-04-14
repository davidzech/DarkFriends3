using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DarkFriendsInjector
{
    public class HookInject
    {
        public static unsafe bool Hook(bool debug)
        {
            string dllPath = Assembly.GetExecutingAssembly().Location;
            dllPath = Path.Combine(Path.GetDirectoryName(dllPath), "DarkFriends3.dll");
            if(!File.Exists(dllPath))
            {
                throw new Exception("DarkFriends3.dll not found");
            }

            IntPtr window = Native.FindWindow(null, "DARK SOULS III");
            if(window.ToInt32() == 0)
            {
                return false;
            }

            uint pid;
            Native.GetWindowThreadProcessId(window, out pid);
            if(pid == 0)
            {
                throw new Exception("Failed to get pid for window");
            }

            byte[] buf = new byte[1];
            buf[0] = debug == true ? (byte)1 : (byte)0;
            fixed(void *ptr = &buf[0])
            {
                EasyHook.NativeAPI.RhInjectLibrary((int)pid, 0, EasyHook.NativeAPI.EASYHOOK_INJECT_DEFAULT, null, dllPath, new IntPtr(ptr), 1);
            }

            return true;            
        }

        public static bool Unhook()
        {
            throw new NotImplementedException();
        }
    }
}
