using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DotNetDemo
{
    public class PerfInfo
    {
        public double timeSoFar;
        List<double> times;

        public PerfInfo()
        {
            timeSoFar = 0.0;
            times = new List<double>(80000);
            times.Add(timeSoFar);
        }

        public void Tick(double ms)
        {
            timeSoFar += ms;
            times.Add(timeSoFar);
        }

        public void StopAndPrint()
        {
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(@"./DotNetDemoTimes.csv"))
            {
                for (int i = 0; i < times.Count; ++i)
                {
                    file.WriteLine(i + "," + times[i]);
                }
            }
        }
    }
}
