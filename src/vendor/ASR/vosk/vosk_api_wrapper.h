#ifndef VOSK_API_WRAPPER_H
#define VOSK_API_WRAPPER_H

#include "vosk_api_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXTERN_FUNC(func) extern fptr_##func func
EXTERN_FUNC(vosk_model_new);
EXTERN_FUNC(vosk_model_free);
EXTERN_FUNC(vosk_model_find_word);
EXTERN_FUNC(vosk_spk_model_new);
EXTERN_FUNC(vosk_spk_model_free);
EXTERN_FUNC(vosk_recognizer_new);
EXTERN_FUNC(vosk_recognizer_new_spk);
EXTERN_FUNC(vosk_recognizer_new_grm);
EXTERN_FUNC(vosk_recognizer_set_spk_model);
EXTERN_FUNC(vosk_recognizer_set_max_alternatives);
EXTERN_FUNC(vosk_recognizer_set_words);
EXTERN_FUNC(vosk_recognizer_accept_waveform);
EXTERN_FUNC(vosk_recognizer_accept_waveform_s);
EXTERN_FUNC(vosk_recognizer_accept_waveform_f);
EXTERN_FUNC(vosk_recognizer_result);
EXTERN_FUNC(vosk_recognizer_partial_result);
EXTERN_FUNC(vosk_recognizer_final_result);
EXTERN_FUNC(vosk_recognizer_reset);
EXTERN_FUNC(vosk_recognizer_free);
EXTERN_FUNC(vosk_set_log_level);
EXTERN_FUNC(vosk_gpu_init);
EXTERN_FUNC(vosk_gpu_thread_init);

#undef EXTERN_FUNC

int vosk_wrapper_init();

#ifdef __cplusplus
}
#endif

#endif