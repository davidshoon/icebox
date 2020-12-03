
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
//#include <math.h>

const int Xlen = 16;
const int Hlen = 4;
const int Olen = 2;
const double MU = 0.5;
const double THRESHOLD = 0.95;

double table[][Xlen] = 
{
  {
    0., 0., 0., 0., 
    1., 1., 1., 1.,
    0., 0., 1., 0.,
    0., 0., 1., 0.,
  },
  {
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.,
  }
};

double X[Xlen]; // input layer
double WH[Xlen][Hlen]; // weight input<->hidden
double H[Hlen]; // hidden layer
double WO[Hlen][Olen]; // weight hidden<->output 
double O[Olen]; // output layer

double sigmoid(double x)
{
  return 1 / (1 + exp(-x));
}

// Calculate nodes between an input and output layer.
// input and output can be a 1D array, or a std::vector or similar.
// weight can be a 2D array, std::vector of a std::vector, or similar.
// whereby, weight[row][col] is used to access elements,
// and row = input_size, col = output_size
template <class _VectorI, class _Matrix, class _VectorO>
void calculate( _VectorI input, unsigned input_size, 
        _Matrix weight,
        _VectorO output, unsigned output_size)
{
  double sum;

  for (unsigned i = 0; i < output_size; i++)
  {
    sum = 0.0;
    for (unsigned j = 0; j < input_size; j++)
    {
      sum += input[j] * weight[j][i];
    }
    output[i] = sigmoid(sum);
  }
}

// returns a random value between 0.0 and 1.0
double double_rand()
{
  return static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
}


// randomize a weight matrix
template <class _Matrix>
void randomize_weights(_Matrix weight, unsigned rows, unsigned cols, 
             double minval, double maxval)
{
  double range = maxval - minval;

  for (unsigned i = 0; i < cols; i++)
  {
    for (unsigned j = 0; j < rows; j++)
    {
      weight[j][i] = double_rand() * range - minval;
    }
  }
}

template <class _Vector>
void randomize_vector(_Vector input, unsigned input_size, double probability)
{
  for (unsigned i = 0; i < input_size; i++)
  {
    double r = double_rand();

    if (r < probability)
    {
      input[i] = fabs(1.0 - input[i]);
    }
  }
}

template <  class _VectorI, class _VectorH, class _VectorO, 
      class _MatrixIH, class _MatrixHO, class _VectorE >
void adjust_weights(_VectorI input, unsigned input_size, 
          _VectorH hidden, unsigned hidden_size,
          _VectorO output, unsigned output_size,
          _MatrixIH weight_ih,
          _MatrixHO weight_ho,
          _VectorE error, /* error_size should be output_size */
          double mu)
{
  for (unsigned i = 0; i < output_size; i++)
  {
    double slope_output = output[i] * (1 - output[i]);

    for (unsigned j = 0; j < hidden_size; j++)
    {
      double slope_hidden = hidden[j] * (1 - hidden[j]);

      for (unsigned k = 0; k < input_size; k++)
      {
        double dx3dw_ih = input[k] * slope_hidden * weight_ho[j][i] * slope_output;
        weight_ih[k][j] += dx3dw_ih * error[i] * mu;
      }

      double dx3dw_ho = hidden[j] * slope_output;
      weight_ho[j][i] += dx3dw_ho * error[i] * mu;
    }
  }
}

template <class _Vector>
void zero_vector(_Vector v, unsigned len)
{
  for (unsigned i = 0; i < len; i++)
  {
    v[i] = 0.0;
  }
}

template <class _VectorA, class _VectorB>
void copy_vector(_VectorA dst, _VectorB src, unsigned len)
{
  for (unsigned i = 0; i < len; i++)
  {
    dst[i] = src[i];
  }
}

int my_start()
{
  //srand(static_cast <unsigned> (time(NULL)));
  srand(0);

  Serial.println("Randomizing weights...");

  randomize_weights(WH, Xlen, Hlen, -0.0005, 0.0005);
  randomize_weights(WO, Hlen, Olen, -0.5, 0.5);

  // training loop
  double E[Olen] = {0};
  double ESum[Olen] = {0};
  Serial.println("Training neural network...");
  for (int i = 0; i < 800; i++)
  {
    zero_vector(ESum, Olen);

    for (int j = 0; j < Olen; j++)
    {
      copy_vector(X, table[j], Xlen);
      randomize_vector(X, Xlen, 0.10);

      calculate(X, Xlen, WH, H, Hlen);
      calculate(H, Hlen, WO, O, Olen);

      for (int k = 0; k < Olen; k++)
      {
        if (j == k)
          E[k] = 5.0 * (1.0 - O[k]);

        else
          E[k] = 0.0 - O[k];

        ESum[k] += E[k] * E[k];
      }

      adjust_weights(X, Xlen, H, Hlen, O, Olen, WH, WO, E, MU);
    }

    /*
    for (int k = 0; k < Olen; k++)
    {
      printf("ESum %d: %f\n", k, ESum[k]);
    }
    */
  }
  Serial.println("\n\n");

  // test loop
  for (int j = 0; j < Olen; j++)
  {
    copy_vector(X, table[j], Xlen);
    randomize_vector(X, Xlen, 0.10);

    calculate(X, Xlen, WH, H, Hlen);
    calculate(H, Hlen, WO, O, Olen);

    Serial.print("====== Test vector ");
    Serial.print(j);
    Serial.println(" ======");

    for (int i = 0; i < Xlen; i++)
    {
      if (i % 4 == 0)
        Serial.println("");

      Serial.print(X[i]);
      Serial.print(" ");
    }

    Serial.println("");
    int selected = -1;
    for (int i = 0; i < Olen; i++)
    {
      Serial.print("O = ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(O[i]);
      if (O[i] > THRESHOLD)
      {
        selected = i;
      }
    }

    Serial.print("Neural network selected ");
    Serial.println(selected);
  }

  return 0;
}

void setup() {
  // put your setup code here, to run once:
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // prints title with ending line break
  Serial.println("Running NN...");
}

void loop() {
  // put your main code here, to run repeatedly:
  my_start();
}
