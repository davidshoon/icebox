/*

	Fractal generator code

	By David Shoon 


*/

import java.awt.*;
import javax.swing.*;

class Complex
{
        // real part x, imaginary part y
        private double x, y;

        // constructors
        public Complex()
        {
                set(0, 0);
        }

        public Complex(double x, double y)
        {
                set(x, y);
        }

        public Complex(Complex a)
        {
                set(a);
        }

        public void set(double x, double y)
        {
                this.x = x;
                this.y = y;
        }

        public void set(Complex a)
        {
                this.x = a.x;
                this.y = a.y;
        }

        public void setX(double a) { x = a; }
        public void setY(double a) { y = a; }

        public double getX()
        {
                return x;
        }

        public double getY()
        {
                return y;
        }

        public void add(Complex a, Complex b)
        {
                this.x = a.getX() + b.getX();
                this.y = a.getY() + b.getY();
        }

        public void sub(Complex a, Complex b)
        {
                this.x = a.getX() - b.getX();
                this.y = a.getY() - b.getY();
        }

        public void multiply(Complex a, Complex b)
        {
                this.x = a.getX() * b.getX() - a.getY() * b.getY();
                this.y = a.getX() * b.getY() + b.getX() * a.getY();
        }

        public void square(Complex a)
        {
                multiply(a, a);
        }

        public void pow(Complex a, int n)
        {
                Complex temp = new Complex();

                for (int i = 1; i < n; i++) {
                        temp.multiply(a, a);
                        a.set(temp);
                }

                set(a);
        }

        public double magnitude()
        {
                return Math.sqrt(x * x + y * y);
        }
}

// colour ramps
class CR_classic
{
        public Color ramp(int x, int y, int loops, int max) {
                if (loops > 255) loops = 255;
                return new Color(loops,loops,loops);
        }

}

class CR_classic_inverse extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max) {
                max = 255;
                if (loops > 255) loops = 255;
                loops = max - loops;
                return new Color(loops,loops,loops);
        }

}

class CR_classic_RGB extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max) {
                return new Color(loops);
        }
}

class CR_classic_RGB_inverse extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max) {
                return new Color(0 - loops);
        }

}

class CR_classic_redscale extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max)
        {
                float ratio;

                ratio = (float) (max - loops) / (float) max;

                if (loops >= max)
                        return new Color(0, 0, 0);
                else
                        return new Color(ratio, 0, 0);
        }
}

class CR_classic_redscale_square extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max)
        {
                float ratio;

                ratio = (float) (max - (loops * loops)) / (float) max;
                if (ratio >= 1.0) ratio = (float) 1.0;
                if (ratio <= 0.0) ratio = (float) 0.0;

                if (loops >= max)
                        return new Color(0, 0, 0);
                else
                        return new Color(ratio, 0, 0);
        }
}

class CR_classic_redscale_log extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max)
        {
                double value;

                if (loops > max) loops = max;

                value = (double) (max - loops);

                if (value != 0.0) {
                        value = Math.log(value);
                        value = 1.0 / value;
                }

                return new Color((float) value, 0, 0);
        }
}

class CR_classic_bluescale_square extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max)
        {
                float ratio;

                ratio = (float) (max - (loops * loops)) / (float) max;
                if (ratio >= 1.0) ratio = (float) 1.0;
                if (ratio <= 0.0) ratio = (float) 0.0;

                if (loops >= max)
                        return new Color(0, 0, 0);
                else
                        return new Color(0, 0, ratio);
        }
}

class CR_classic_boundary extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max)
        {
                if (loops <= 100)
                        return new Color(0, 0, 255);

                else 
                        return new Color(0, 0, 0);
        }
}

class CR_classic_hsv_hue_simple extends CR_classic
{
        public Color ramp(int x, int y, int loops, int max)
        {
                return new Color(
                        Color.HSBtoRGB((float) loops / (float) 360.0 + (float) 0.5, (float) 1.0, (float) 1.0)
                        );
        }
}

class CR_classic_hsv_value extends CR_classic
{
        private float hue, saturation;

        public CR_classic_hsv_value(double hue, double saturation)
        {
                this.hue = (float) hue;
                this.saturation = (float) saturation;
        }

        public Color ramp(int x, int y, int loops, int max)
        {
                max = 50;

                if (loops > max) loops = max;

                loops = max - loops;

                return new Color(
                        Color.HSBtoRGB(hue, saturation, (float) loops / (float)max)
                        );
        }
}


class Fractal extends JFrame
{
	Image image;
	Graphics gfx;
        int ok = 0;

        public int loop_zsquare_plus_c(Complex z_init, Complex c, int max)
        {
                Complex z = new Complex();
                int i;
                Complex temp = new Complex();
                double max_mag;

                // under Hearn&Baker, they check max_mag up to 2,
                // under lecturer's notes, it checks for large M.
                // because of the z^2 function, a suitably large M
                // should be ok, and produces a better image.
                max_mag = 200.0;

                z.set(z_init);

                for (i = 0; (z.magnitude() <= max_mag) && (i < max); i++) {
                        temp.square(z);
                        z.add(temp, c);
                }

                return i;
        }

        public void classic(Graphics g,
                        int max, int width, int height,
                        Complex z_init, Complex range_min, Complex range_max,
                        CR_classic cr)
        {
                Complex c, inc;
                int loops;
                int x, y;

                c = new Complex();
                inc = new Complex();             

                inc.sub(range_max, range_min);
                inc.set(inc.getX() / (double) width, inc.getY() / (double) height);

                for (y = 0, c.setY(range_min.getY()); y < height; y++, c.setY(c.getY() + inc.getY()))
                {
                        for (x = 0, c.setX(range_min.getX()); x < width; x++, c.setX(c.getX() + inc.getX()))
                        {
                                loops = loop_zsquare_plus_c(z_init, c, max);
                                g.setColor(cr.ramp(x, y, loops, max));
                                g.drawLine(x, y, x, y);
                        }
                }
        }


        public int loop_zn_plus_c(Complex z_init, Complex c, int max, int n)
        {
                Complex z = new Complex();
                int i;
                Complex temp = new Complex();
                double max_mag;
                max_mag = 200.0;

                z.set(z_init);

                for (i = 0; (z.magnitude() <= max_mag) && (i < max); i++) {
                        temp.pow(z, n);
                        z.add(temp, c);
                }

                return i;
        }


        public void zn(Graphics g,
                        int max, int width, int height,
                        Complex z_init, Complex range_min, Complex range_max,
                        CR_classic cr, int n)
        {
                Complex c, inc;
                int loops;
                int x, y;

                c = new Complex();
                inc = new Complex();             

                inc.sub(range_max, range_min);
                inc.set(inc.getX() / (double) width, inc.getY() / (double) height);

                for (y = 0, c.setY(range_min.getY()); y < height; y++, c.setY(c.getY() + inc.getY()))
                {
                        for (x = 0, c.setX(range_min.getX()); x < width; x++, c.setX(c.getX() + inc.getX()))
                        {
                                loops = loop_zn_plus_c(z_init, c, max, n);
                                g.setColor(cr.ramp(x, y, loops, max));
                                g.drawLine(x, y, x, y);
                        }
                }
        }


	public void paint(Graphics g)
	{
                if (ok != 0)
                        g.drawImage(image, 40, 40, null);
	}

        public void mandelbrot(Graphics g, 
                        int max, int width, int height, CR_classic cr)
        {

                // mandelbrot set.
                classic(g, max, width, height, new Complex(0, 0), 
                        new Complex(-2.5, -1.4), new Complex(1.5, 1.4), cr);

        }

        public void sleep(int n)
        {
                try {
                        Thread.sleep(n);
                } catch (Exception e) { }
        }

        public Fractal()
	{
		setSize(660, 500);
		setVisible(true);

		image = createImage(800,600);			

		gfx = image.getGraphics();

                ok = 1;


while (true) 
{

                // mandelbrot set.

                mandelbrot(gfx, 1000, 600, 600, new CR_classic()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_inverse()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 6600, 600, new CR_classic_RGB()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_RGB_inverse());repaint();  sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_redscale()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_redscale_square()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_redscale_log()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_bluescale_square()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_boundary()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_hsv_hue_simple()); repaint(); sleep(1000);
                mandelbrot(gfx, 1000, 600, 600, new CR_classic_hsv_value(0.567, 0.877)); repaint(); sleep(1000);


                // just a julia set of some sort.. nice ones: (1, 0),
                classic(gfx, 1000, 600, 600, new Complex(1, 0), 
                        new Complex(-2.25, -1.25), new Complex(0.75, 1.25),
                        new CR_classic_redscale_square());
                repaint(); sleep(1000);

                // now run for n = 1 to 9:
                // run z <- z^n + c


                // when n = 1, it looks much more better with CR_classic()
                zn(gfx, 1000, 600, 600, new Complex(0, 0),
                        new Complex(-3, -3), new Complex(3, 3),
                        new CR_classic(), 1);
                repaint(); sleep(1000);


                for (int i = 2; i < 10; i++) {

                        zn(gfx, 1000, 600, 600, new Complex(0, 0),
                                new Complex(-3, -3), new Complex(3, 3),
                                new CR_classic_hsv_value(0.567, 0.877), i);


                        repaint();
                        sleep(1000);
                }
}
	}

	public static void main(String args[]) {
                new Fractal();
	}

}
