using System;

namespace DotNetDemo
{
#if WINDOWS || XBOX
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main(string[] args)
        {
            using (CSharpDemo game = new CSharpDemo())
            {
                game.Run();
            }
        }
    }
#endif
}

