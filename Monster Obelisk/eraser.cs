using System;
using System.Drawing;
using System.Drawing.Imaging;

class Program
{
    static void Main(string[] args)
    {
        if (args.Length == 0) return;
        string file = args[0];
        Console.WriteLine("Processing " + file);
        Bitmap bmp = new Bitmap(file);
        
        // We will define "checkered background" by sampling the top-left corner
        // Usually it's white (255,255,255) and a light gray (like 204,204,204)
        // We'll consider any very light gray or white as the background.
        // Actually, let's just sample the 4x4 area in the top left.
        
        Color c1 = bmp.GetPixel(0, 0);
        Color c2 = bmp.GetPixel(1, 0);
        if (c1 == c2) {
            c2 = bmp.GetPixel(10, 0); // try finding the other checker color
            if (c1 == c2) c2 = bmp.GetPixel(0, 10);
        }

        Console.WriteLine("Checker colors found: {0} and {1}", c1, c2);
        
        for (int y = 0; y < bmp.Height; y++)
        {
            for (int x = 0; x < bmp.Width; x++)
            {
                Color p = bmp.GetPixel(x, y);
                // The checkerboard usually has R,G,B roughly equal and within a specific range (e.g., > 180).
                // Let's use a tolerance to erase both colours and their compressed artifacts.
                bool match1 = Math.Abs(p.R - c1.R) < 25 && Math.Abs(p.G - c1.G) < 25 && Math.Abs(p.B - c1.B) < 25;
                bool match2 = Math.Abs(p.R - c2.R) < 25 && Math.Abs(p.G - c2.G) < 25 && Math.Abs(p.B - c2.B) < 25;
                
                if (match1 || match2)
                {
                    bmp.SetPixel(x, y, Color.Transparent);
                }
            }
        }
        
        string outFile = file + ".tmp.png";
        bmp.Save(outFile, ImageFormat.Png);
        bmp.Dispose();
        
        // Overwrite
        System.IO.File.Delete(file);
        System.IO.File.Move(outFile, file);
        Console.WriteLine("Done.");
    }
}
