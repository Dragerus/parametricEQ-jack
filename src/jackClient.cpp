#include <jack/jack.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <algorithm>

jack_port_t *inPort;
jack_port_t *outPort;
jack_client_t *jackClient;
jack_nframes_t meanSamplesCount = 128;
jack_default_audio_sample_t meanWindow[129];

int processAudio(jack_nframes_t nframes, void *arg) {
	jack_default_audio_sample_t *inBuffer, *outBuffer;
	inBuffer = (jack_default_audio_sample_t *)jack_port_get_buffer(inPort, nframes);
	outBuffer = (jack_default_audio_sample_t *)jack_port_get_buffer(outPort, nframes);

	jack_default_audio_sample_t sampleSum = 0;
	jack_nframes_t i = 0, j = 0;
	while(i < nframes) {
		sampleSum = 0.0;
		for(j = 0; j < meanSamplesCount; j++) {
			sampleSum += meanWindow[j];
			meanWindow[j] = meanWindow[j + 1];
		}
		outBuffer[i] = sampleSum / meanSamplesCount;
		meanWindow[meanSamplesCount] = inBuffer[i];
		i++;
	}

	//printf("\r% .5f", sampleSum / nframes);
	//printf("\r% .5f", sampleMax);
	//printf("\r%d", nframes);

	return 0;
}

void jackServerOnShutdown(void *arg) {
	jack_client_close(jackClient);
	std::cerr << "Jack server shut down" << std::endl;
	std::exit(1);
}

int main(int argc, char** argv) {
	const char *clientName = "Parametric EQ";
	const char *serverName = nullptr;
	jack_options_t jackOptions = JackNullOption;
	jack_status_t jackStatus;

	jackClient = jack_client_open(clientName, jackOptions, &jackStatus, serverName);
	
	jack_set_process_callback(jackClient, processAudio, nullptr);
	jack_on_shutdown(jackClient, jackServerOnShutdown, nullptr);

	std::printf(
		"Jack audio properties:\n"\
		"Sample rate: %d\n"\
		"Buffer size: %d\n",
		jack_get_sample_rate(jackClient),
		jack_get_buffer_size(jackClient)
	);

	inPort = jack_port_register(
		jackClient,
		"input",
		JACK_DEFAULT_AUDIO_TYPE,
		JackPortIsInput,
		0
	);
	outPort = jack_port_register(
		jackClient,
		"output",
		JACK_DEFAULT_AUDIO_TYPE,
		JackPortIsOutput,
		0
	);

	jack_activate(jackClient);

	getchar();

	jack_client_close(jackClient);

	return 0;
}
