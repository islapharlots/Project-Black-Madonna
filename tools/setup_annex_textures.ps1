param(
    [string]$Root = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($Root)) {
    $Root = Split-Path $PSScriptRoot -Parent
}

$Root = [System.IO.Path]::GetFullPath($Root)
$TextureDir = Join-Path $Root "stbrielle\textures\stbrielle\annex"
$Marker = Join-Path $TextureDir ".generated-v1"
$Version = "municipal-annex-textures-v1-2026-10-17"

$Required = @(
    "plaster_d.tga", "plaster_local.tga", "plaster_s.tga",
    "tile_d.tga", "tile_local.tga", "tile_s.tga",
    "stone_d.tga", "stone_local.tga", "stone_s.tga",
    "blackmetal_d.tga", "blackmetal_local.tga", "blackmetal_s.tga",
    "wood_d.tga", "wood_local.tga", "wood_s.tga",
    "brass_d.tga", "brass_local.tga", "brass_s.tga",
    "paint_d.tga", "paint_local.tga", "paint_s.tga"
)

$Ready = $false
if (Test-Path $Marker) {
    $MarkerValue = (Get-Content -Raw -Path $Marker).Trim()
    if ($MarkerValue -eq $Version) {
        $Ready = $true
        foreach ($File in $Required) {
            if (-not (Test-Path (Join-Path $TextureDir $File))) {
                $Ready = $false
                break
            }
        }
    }
}

if ($Ready) {
    Write-Host "[OK] Municipal Annex texture set already generated."
    exit 0
}

New-Item -ItemType Directory -Force -Path $TextureDir | Out-Null

$Source = @'
using System;
using System.IO;

public static class StBrielleAnnexTextures
{
    const int N = 256;

    static double Clamp(double v, double lo, double hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    static byte B(double v)
    {
        return (byte)Math.Round(Clamp(v, 0.0, 1.0) * 255.0);
    }

    static uint Hash(int x, int y, int seed)
    {
        unchecked
        {
            uint h = (uint)(x * 374761393 + y * 668265263 + seed * 1442695041);
            h = (h ^ (h >> 13)) * 1274126177u;
            return h ^ (h >> 16);
        }
    }

    static double Rand(int x, int y, int seed)
    {
        return (Hash(x, y, seed) & 0x00FFFFFF) / 16777215.0;
    }

    static double Fade(double t)
    {
        return t * t * (3.0 - 2.0 * t);
    }

    static int Wrap(int v, int m)
    {
        v %= m;
        return v < 0 ? v + m : v;
    }

    static double Noise(int x, int y, int cells, int seed)
    {
        double gx = ((double)x / N) * cells;
        double gy = ((double)y / N) * cells;
        int x0 = (int)Math.Floor(gx);
        int y0 = (int)Math.Floor(gy);
        double tx = Fade(gx - Math.Floor(gx));
        double ty = Fade(gy - Math.Floor(gy));

        double a = Rand(Wrap(x0, cells), Wrap(y0, cells), seed);
        double b = Rand(Wrap(x0 + 1, cells), Wrap(y0, cells), seed);
        double c = Rand(Wrap(x0, cells), Wrap(y0 + 1, cells), seed);
        double d = Rand(Wrap(x0 + 1, cells), Wrap(y0 + 1, cells), seed);

        double ab = a + (b - a) * tx;
        double cd = c + (d - c) * tx;
        return ab + (cd - ab) * ty;
    }

    static double Fbm(int x, int y, int seed)
    {
        return Noise(x, y, 4, seed) * 0.50 +
               Noise(x, y, 8, seed + 17) * 0.28 +
               Noise(x, y, 16, seed + 31) * 0.15 +
               Noise(x, y, 32, seed + 47) * 0.07;
    }

    static void Put(byte[] rgb, int x, int y, double r, double g, double b)
    {
        int i = (y * N + x) * 3;
        rgb[i + 0] = B(r);
        rgb[i + 1] = B(g);
        rgb[i + 2] = B(b);
    }

    static void WriteTga(string path, byte[] rgb)
    {
        byte[] header = new byte[18];
        header[2] = 2;
        header[12] = (byte)(N & 255);
        header[13] = (byte)(N >> 8);
        header[14] = (byte)(N & 255);
        header[15] = (byte)(N >> 8);
        header[16] = 24;
        header[17] = 0x20;

        using (FileStream fs = new FileStream(path, FileMode.Create, FileAccess.Write, FileShare.None))
        {
            fs.Write(header, 0, header.Length);
            byte[] row = new byte[N * 3];
            for (int y = 0; y < N; y++)
            {
                for (int x = 0; x < N; x++)
                {
                    int src = (y * N + x) * 3;
                    int dst = x * 3;
                    row[dst + 0] = rgb[src + 2];
                    row[dst + 1] = rgb[src + 1];
                    row[dst + 2] = rgb[src + 0];
                }
                fs.Write(row, 0, row.Length);
            }
        }
    }

    static void WriteMaps(string dir, string name, byte[] diffuse, double[] height, double[] spec, double normalStrength)
    {
        byte[] normal = new byte[N * N * 3];
        byte[] specRgb = new byte[N * N * 3];

        for (int y = 0; y < N; y++)
        {
            int ym = Wrap(y - 1, N);
            int yp = Wrap(y + 1, N);
            for (int x = 0; x < N; x++)
            {
                int xm = Wrap(x - 1, N);
                int xp = Wrap(x + 1, N);
                double dx = (height[y * N + xp] - height[y * N + xm]) * normalStrength;
                double dy = (height[yp * N + x] - height[ym * N + x]) * normalStrength;
                double nx = -dx;
                double ny = -dy;
                double nz = 1.0;
                double len = Math.Sqrt(nx * nx + ny * ny + nz * nz);
                nx /= len; ny /= len; nz /= len;
                Put(normal, x, y, nx * 0.5 + 0.5, ny * 0.5 + 0.5, nz * 0.5 + 0.5);
                double sv = Clamp(spec[y * N + x], 0.0, 1.0);
                Put(specRgb, x, y, sv, sv, sv);
            }
        }

        WriteTga(Path.Combine(dir, name + "_d.tga"), diffuse);
        WriteTga(Path.Combine(dir, name + "_local.tga"), normal);
        WriteTga(Path.Combine(dir, name + "_s.tga"), specRgb);
    }

    static void BuildPlaster(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double n = Fbm(x, y, 101); double fine = Noise(x, y, 32, 177);
            double patch = Math.Sin((x / (double)N) * Math.PI * 4.0 + Math.Sin((y / (double)N) * Math.PI * 2.0) * 0.7) * 0.035;
            double v = (n - 0.5) * 0.15 + (fine - 0.5) * 0.035 + patch;
            double grime = Math.Pow(0.5 + 0.5 * Math.Sin((y / (double)N) * Math.PI * 2.0 - 1.6), 8.0);
            Put(d, x, y, 0.39 + v - grime * 0.035, 0.40 + v * 0.98 - grime * 0.032, 0.38 + v * 0.92 - grime * 0.028);
            h[y * N + x] = n * 0.58 + fine * 0.10;
            s[y * N + x] = 0.10 + (1.0 - fine) * 0.07 - grime * 0.03;
        }
        WriteMaps(dir, "plaster", d, h, s, 2.6);
    }

    static void BuildTile(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double n = Fbm(x, y, 201); double fine = Noise(x, y, 32, 231); double wear = Noise(x, y, 4, 249);
            int mx = x % 64; int my = y % 64;
            double seam = (mx <= 1 || mx >= 63 || my <= 1 || my >= 63) ? 1.0 : 0.0;
            double v = (n - 0.5) * 0.10 + (fine - 0.5) * 0.025 + (wear - 0.5) * 0.025;
            Put(d, x, y, 0.11 + v - seam * 0.045, 0.125 + v - seam * 0.040, 0.12 + v - seam * 0.035);
            h[y * N + x] = n * 0.48 + fine * 0.08 - seam * 0.22;
            s[y * N + x] = 0.17 + wear * 0.18 - seam * 0.13;
        }
        WriteMaps(dir, "tile", d, h, s, 2.8);
    }

    static void BuildStone(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double n = Fbm(x, y, 301); double fine = Noise(x, y, 32, 337); double mineral = Noise(x, y, 8, 353);
            double v = (n - 0.5) * 0.20 + (fine - 0.5) * 0.055;
            Put(d, x, y, 0.41 + v - (mineral - 0.5) * 0.020, 0.38 + v * 0.95 - (mineral - 0.5) * 0.012, 0.33 + v * 0.85);
            h[y * N + x] = n * 0.67 + fine * 0.11;
            s[y * N + x] = 0.08 + (1.0 - n) * 0.055;
        }
        WriteMaps(dir, "stone", d, h, s, 2.7);
    }

    static void BuildBlackMetal(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double n = Fbm(x, y, 401); double fine = Noise(x, y, 32, 433); double rustBase = Fbm(x, y, 451);
            double rust = Clamp((rustBase - 0.67) * 4.2, 0.0, 1.0);
            double scratch = ((Hash(x, 0, 477) % 97) == 0 && ((y + (int)(Hash(x, 1, 479) % 70)) % 113) < 44) ? 1.0 : 0.0;
            double v = (n - 0.5) * 0.045 + (fine - 0.5) * 0.014;
            Put(d, x, y, 0.043 + v + rust * 0.105 + scratch * 0.040, 0.050 + v + rust * 0.030 + scratch * 0.038, 0.051 + v + rust * 0.006 + scratch * 0.034);
            h[y * N + x] = n * 0.20 + rust * 0.08 - scratch * 0.11;
            s[y * N + x] = 0.31 + n * 0.20 - rust * 0.20 + scratch * 0.13;
        }
        WriteMaps(dir, "blackmetal", d, h, s, 1.9);
    }

    static void BuildWood(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double warp = (Noise(x, y, 4, 501) - 0.5) * 7.0;
            double g1 = 0.5 + 0.5 * Math.Sin(((x + warp) / N) * Math.PI * 2.0 * 22.0);
            double g2 = 0.5 + 0.5 * Math.Sin(((x + warp) / N) * Math.PI * 2.0 * 47.0 + 1.2);
            double fine = Noise(x, y, 32, 529);
            double val = g1 * 0.34 + g2 * 0.10 + fine * 0.20 + Noise(x, y, 8, 547) * 0.36;
            double v = (val - 0.5) * 0.10;
            Put(d, x, y, 0.105 + v, 0.042 + v * 0.55, 0.024 + v * 0.32);
            h[y * N + x] = val * 0.34 + fine * 0.07;
            s[y * N + x] = 0.10 + val * 0.13;
        }
        WriteMaps(dir, "wood", d, h, s, 2.3);
    }

    static void BuildBrass(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double n = Fbm(x, y, 601); double fine = Noise(x, y, 32, 619); double tarn = Clamp((Fbm(x, y, 641) - 0.57) * 3.0, 0.0, 1.0);
            double v = (n - 0.5) * 0.15 + (fine - 0.5) * 0.025;
            double r = (0.37 + v) * (1.0 - tarn * 0.28) + 0.035 * tarn;
            double g = (0.23 + v * 0.75) * (1.0 - tarn * 0.28) + 0.085 * tarn;
            double b = (0.075 + v * 0.25) * (1.0 - tarn * 0.28) + 0.055 * tarn;
            Put(d, x, y, r, g, b);
            h[y * N + x] = n * 0.18 + fine * 0.04 + tarn * 0.05;
            s[y * N + x] = 0.47 + n * 0.18 - tarn * 0.34;
        }
        WriteMaps(dir, "brass", d, h, s, 1.8);
    }

    static void BuildPaint(string dir)
    {
        byte[] d = new byte[N * N * 3]; double[] h = new double[N * N]; double[] s = new double[N * N];
        for (int y = 0; y < N; y++) for (int x = 0; x < N; x++)
        {
            double n = Fbm(x, y, 701); double fine = Noise(x, y, 32, 727);
            double v = 0.68 + (n - 0.5) * 0.18 + (fine - 0.5) * 0.04;
            Put(d, x, y, v, v, v);
            h[y * N + x] = n * 0.48 + fine * 0.08;
            s[y * N + x] = 0.13 + (1.0 - n) * 0.08;
        }
        WriteMaps(dir, "paint", d, h, s, 2.4);
    }

    public static void Generate(string dir)
    {
        Directory.CreateDirectory(dir);
        BuildPlaster(dir);
        BuildTile(dir);
        BuildStone(dir);
        BuildBlackMetal(dir);
        BuildWood(dir);
        BuildBrass(dir);
        BuildPaint(dir);
    }
}
'@

try {
    Write-Host "[BUILD] Generating original Municipal Annex texture set..."
    Add-Type -TypeDefinition $Source -Language CSharp
    [StBrielleAnnexTextures]::Generate($TextureDir)
    Set-Content -Path $Marker -Value $Version -Encoding ASCII

    foreach ($File in $Required) {
        $Path = Join-Path $TextureDir $File
        if (-not (Test-Path $Path)) {
            throw "Texture generation did not create $File"
        }
    }

    Write-Host "[OK] Municipal Annex textures generated in $TextureDir"
    exit 0
}
catch {
    Write-Host "[ERROR] Municipal Annex texture generation failed." -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
