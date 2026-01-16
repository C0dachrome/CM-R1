#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>
#include <iio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sock;
struct sockaddr_in laptop_addr;

#define PI 3.14159265358979323846
#define FFT_SIZE 256

// Simple Complex struct for our manual FFT
typedef struct
{
	float real;
	float imag;
} complex_t;

// Minimalist FFT implementation
void fft(complex_t *v, int n, complex_t *tmp)
{
	if (n > 1)
	{
		int i, k;
		complex_t z;
		for (i = 0; i < n / 2; i++)
		{
			tmp[i] = v[2 * i];
			tmp[i + n / 2] = v[2 * i + 1];
		}
		fft(tmp, n / 2, v);
		fft(tmp + n / 2, n / 2, v);
		for (k = 0; k < n / 2; k++)
		{
			float angle = -2 * PI * k / n;
			complex_t t = {cosf(angle), sinf(angle)};
			z.real = t.real * tmp[k + n / 2].real - t.imag * tmp[k + n / 2].imag;
			z.imag = t.real * tmp[k + n / 2].imag + t.imag * tmp[k + n / 2].real;
			v[k].real = tmp[k].real + z.real;
			v[k].imag = tmp[k].imag + z.imag;
			v[k + n / 2].real = tmp[k].real - z.real;
			v[k + n / 2].imag = tmp[k].imag - z.imag;
		}
	}
}

int main()
{
	// Master (Local)
	struct iio_context *ctx_rx = iio_create_local_context();

	// Slave (Network)
	struct iio_context *ctx_tx = iio_create_context_from_uri("ip:192.168.2.51"); // create context from uri is more robust

	// if (!ctx_tx)
	// {
	// 	fprintf(stderr, "ERROR: Context creation failed. Is the IP correct?\n");
	// 	return -1;
	// }

	// // Check if the device exists on that specific context
	// struct iio_device *tx_phy = iio_context_find_device(ctx_tx, "ad9361-phy");
	// if (!tx_phy)
	// {
	// 	fprintf(stderr, "ERROR: Found Slave Pluto, but could not find AD9361 chip!\n");
	// 	iio_context_destroy(ctx_tx);
	// 	return -1;
	// }

	if (!ctx_tx)
	{
		printf("Error: Slave Pluto not found at .51\n");
		return -1;
	}

	// --- DEVICES ---
	struct iio_device *rx_phy = iio_context_find_device(ctx_rx, "ad9361-phy");
	struct iio_device *rx_dev = iio_context_find_device(ctx_rx, "cf-ad9361-lpc");
	struct iio_device *tx_phy = iio_context_find_device(ctx_tx, "ad9361-phy");
	struct iio_device *tx_dev = iio_context_find_device(ctx_tx, "cf-ad9361-dds-core-lpc");

	tx_phy = iio_context_find_device(ctx_tx, "ad9361-phy");
	if (!tx_phy)
	{
		printf("Error: Could not find tx_phy\n");
		return -1;
	}

	tx_dev = iio_context_find_device(ctx_tx, "cf-ad9361-dds-core-lpc");
	if (!tx_dev)
	{
		printf("Error: Could not find tx_dev\n");
		return -1;
	}

	// --- CONTEXT CHECKS ---
	if (!ctx_rx)
	{
		printf("CRASH: Master Pluto context is NULL\n");
		return -1;
	}
	if (!ctx_tx)
	{
		printf("CRASH: Slave Pluto context is NULL\n");
		return -1;
	}

	// --- DEVICE CHECKS ---
	rx_phy = iio_context_find_device(ctx_rx, "ad9361-phy");
	if (!rx_phy)
	{
		printf("CRASH: Could not find ad9361-phy on Master\n");
		return -1;
	}

	tx_phy = iio_context_find_device(ctx_tx, "ad9361-phy");
	if (!tx_phy)
	{
		printf("CRASH: Could not find ad9361-phy on Slave\n");
		return -1;
	}

	// --- RADAR PARAMS & CHIRP GEN ---
	float fs = 5000000.0; // 5 MHz
	float bw = 2000000.0; // 2 MHz Sweep
	float T = (float)FFT_SIZE / fs;
	float ref_i[FFT_SIZE], ref_q[FFT_SIZE];

	for (int i = 0; i < FFT_SIZE; i++)
	{
		float t = (float)i / fs;
		float phase = PI * (bw / T) * t * t;
		ref_i[i] = cosf(phase);
		ref_q[i] = sinf(phase);
	}

	// --- HARDWARE SETUP ---
	// Clock Sync: Disable Slave's internal XO so it listens to the Master's clock
	iio_device_attr_write_bool(tx_phy, "xo_disable_use_ext_refclk", true);
	iio_device_attr_write_bool(rx_phy, "xo_disable_use_ext_refclk", false);

	// Set Frequencies
	struct iio_channel *rx_lo = iio_device_find_channel(rx_phy, "altvoltage0", true);
	struct iio_channel *tx_lo = iio_device_find_channel(tx_phy, "altvoltage0", true);
	iio_channel_attr_write_longlong(rx_lo, "frequency", 5800000000LL);
	iio_channel_attr_write_longlong(tx_lo, "frequency", 5800000000LL);

	// Enable RX Channels
	struct iio_channel *rx0_i = iio_device_find_channel(rx_dev, "voltage0", false);
	struct iio_channel *rx0_q = iio_device_find_channel(rx_dev, "voltage1", false);
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);

	// Enable I and Q channels on the Slave Pluto
	struct iio_channel *tx0_i = iio_device_find_channel(tx_dev, "voltage0", true);
	struct iio_channel *tx0_q = iio_device_find_channel(tx_dev, "voltage1", true);

	if (!tx0_i || !tx0_q)
	{
		fprintf(stderr, "ERROR: Could not find TX channels on Slave\n");
		return -1;
	}

	iio_channel_enable(tx0_i);
	iio_channel_enable(tx0_q);

	// pointer / null check
	struct iio_buffer *txbuf = iio_device_create_buffer(tx_dev, FFT_SIZE, true);
	if (!txbuf)
	{
		fprintf(stderr, "ERROR: Failed to create TX buffer\n");
		return -1;
	}

	int16_t *tx_raw = (int16_t *)iio_buffer_start(txbuf);
	if (!tx_raw)
	{
		fprintf(stderr, "ERROR: tx_raw pointer is NULL (Did you enable channels?)\n");
		return -1;
	}

	// --- TX CHIRP PUSH ---
	for (int i = 0; i < FFT_SIZE; i++)
	{
		tx_raw[i * 2] = (int16_t)(ref_i[i] * 2000);
		tx_raw[i * 2 + 1] = (int16_t)(ref_q[i] * 2000);
	}
	iio_buffer_push(txbuf);

	// --- RX BUFFER ---
	struct iio_buffer *rxbuf = iio_device_create_buffer(rx_dev, FFT_SIZE, false);
	complex_t data[FFT_SIZE], tmp[FFT_SIZE];

	printf("FMCW Radar Online. Starting Processing...\n");

	// Inside main(), before the while(1)
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	laptop_addr.sin_family = AF_INET;
	laptop_addr.sin_port = htons(5005);
	laptop_addr.sin_addr.s_addr = inet_addr("192.168.2.0"); // Your laptop's IP

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		perror("Socket creation failed");
		return -1;
	}

	laptop_addr.sin_family = AF_INET;
	laptop_addr.sin_port = htons(5005);
	laptop_addr.sin_addr.s_addr = inet_addr("192.168.2.10");

	if (laptop_addr.sin_addr.s_addr == INADDR_NONE)
	{
		printf("Error: Invalid Laptop IP address\n");
		return -1;
	}

	while (1)
	{
		iio_buffer_refill(rxbuf);
	
		int16_t *raw = (int16_t *)iio_buffer_start(rxbuf);

		for (int i = 0; i < FFT_SIZE; i++)
		{
			float window = 0.5f * (1.0f - cosf(2.0f * PI * i / (FFT_SIZE - 1)));

			// De-interleave RX
			float rx_i = (float)raw[i * 2];
			float rx_q = (float)raw[i * 2 + 1];

			// FMCW MIXING (Complex Conjugate Multiplication)
			// This extracts the "Beat Frequency"
			data[i].real = (rx_i * ref_i[i] + rx_q * ref_q[i]) * window;
			data[i].imag = (rx_q * ref_i[i] - rx_i * ref_q[i]) * window;
		}

		fft(data, FFT_SIZE, tmp);

		// Find strongest bin (ignoring DC in Bin 0)
		int max_bin = 1;
		float max_mag = 0;
		for (int i = 1; i < FFT_SIZE / 2; i++)
		{
			float mag = sqrtf(data[i].real * data[i].real + data[i].imag * data[i].imag);
			if (mag > max_mag)
			{
				max_mag = mag;
				max_bin = i;
			}
		}

		char buffer[2048];
		int len = sprintf(buffer, "DATA:");
		for (int i = 0; i < FFT_SIZE / 2; i++)
		{
			float mag = sqrtf(data[i].real * data[i].real + data[i].imag * data[i].imag);
			len += sprintf(buffer + len, "%.2f%s", mag, (i == (FFT_SIZE / 2 - 1)) ? "" : ",");
		}
		sendto(sock, buffer, len, MSG_DONTWAIT, (struct sockaddr *)&laptop_addr, sizeof(laptop_addr));
	}

	// Cleanup
	iio_buffer_destroy(rxbuf);
	iio_buffer_destroy(txbuf);
	iio_context_destroy(ctx_rx);
	iio_context_destroy(ctx_tx);

	return 0;
}