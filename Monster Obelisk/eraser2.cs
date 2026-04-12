using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections.Generic;
using System.Linq;

class Program
{
    static void Main(string[] args)
    {
        if (args.Length == 0) return;
        string file = args[0];
        Console.WriteLine("Processing " + file);
        Bitmap bmp = new Bitmap(file);
        
        Dictionary<Color, int> borderColors = new Dictionary<Color, int>();
        for (int x = 0; x < bmp.Width; x++) {
            AddColor(borderColors, bmp.GetPixel(x, 0));
            AddColor(borderColors, bmp.GetPixel(x, bmp.Height - 1));
        }
        for (int y = 0; y < bmp.Height; y++) {
            AddColor(borderColors, bmp.GetPixel(0, y));
            AddColor(borderColors, bmp.GetPixel(bmp.Width - 1, y));
        }
        
        var topColors = borderColors.OrderByDescending(kv => kv.Value).Take(2).Select(kv => kv.Key).ToList();
        Color c1 = topColors.Count > 0 ? topColors[0] : Color.White;
        Color c2 = topColors.Count > 1 ? topColors[1] : Color.White;
        
        Console.WriteLine("Dominant border colors: {0} and {1}", c1, c2);
        
        // Also target exact White and standard Grey just in case they're checkered
        Color c3 = Color.FromArgb(255, 255, 255);
        Color c4 = Color.FromArgb(204, 204, 204);
        Color c5 = Color.FromArgb(0, 0, 0); // Remove black as well since they used 0,0,0 earlier

        for (int y = 0; y < bmp.Height; y++)
        {
            for (int x = 0; x < bmp.Width; x++)
            {
                Color p = bmp.GetPixel(x, y);
                // Ignore already transparent
                if (p.A == 0) continue;

                bool isBg = IsSimilar(p, c1) || IsSimilar(p, c2) || IsSimilar(p, c3) || IsSimilar(p, c4) || IsSimilar(p, c5);
                
                // Extra check for "grey" where R, G, B are similar (monochrome)
                bool isMonochrome = Math.Abs(p.R - p.G) < 15 && Math.Abs(p.G - p.B) < 15 && Math.Abs(p.R - p.B) < 15;
                // Checkered backgrounds are usually monochrome grey/white. But we don't want to erase ALL grey inside the UI.
                // We'll stick to erasing the dominant border colors and standard checker colors.

                if (isBg)
                {
                    bmp.SetPixel(x, y, Color.Transparent);
                }
            }
        }
        
        string outFile = file + ".tmp.png";
        bmp.Save(outFile, ImageFormat.Png);
        bmp.Dispose();
        
        System.IO.File.Delete(file);
        System.IO.File.Move(outFile, file);
        Console.WriteLine("Done.");
    }

    static void AddColor(Dictionary<Color, int> dict, Color c) {
        if (c.A < 10) return; // ignore transparent
        foreach (var key in dict.Keys.ToList()) {
            if (IsSimilar(key, c)) {
                dict[key]++;
                return;
            }
        }
        dict[c] = 1;
    }

    static bool IsSimilar(Color c1, Color c2) {
        return Math.Abs(c1.R - c2.R) < 25 && Math.Abs(c1.G - c2.G) < 25 && Math.Abs(c1.B - c2.B) < 25;
    }
}
