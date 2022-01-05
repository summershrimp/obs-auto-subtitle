#include <util/platform.h>
#include "vosk_api_wrapper.h"

#define DEF_FUNC_PTR(func) fptr_##func func
DEF_FUNC_PTR(vosk_model_new);
DEF_FUNC_PTR(vosk_model_free);
DEF_FUNC_PTR(vosk_model_find_word);
DEF_FUNC_PTR(vosk_spk_model_new);
DEF_FUNC_PTR(vosk_spk_model_free);
DEF_FUNC_PTR(vosk_recognizer_new);
DEF_FUNC_PTR(vosk_recognizer_new_spk);
DEF_FUNC_PTR(vosk_recognizer_new_grm);
DEF_FUNC_PTR(vosk_recognizer_set_spk_model);
DEF_FUNC_PTR(vosk_recognizer_set_max_alternatives);
DEF_FUNC_PTR(vosk_recognizer_set_words);
DEF_FUNC_PTR(vosk_recognizer_accept_waveform);
DEF_FUNC_PTR(vosk_recognizer_accept_waveform_s);
DEF_FUNC_PTR(vosk_recognizer_accept_waveform_f);
DEF_FUNC_PTR(vosk_recognizer_result);
DEF_FUNC_PTR(vosk_recognizer_partial_result);
DEF_FUNC_PTR(vosk_recognizer_final_result);
DEF_FUNC_PTR(vosk_recognizer_reset);
DEF_FUNC_PTR(vosk_recognizer_free);
DEF_FUNC_PTR(vosk_set_log_level);
DEF_FUNC_PTR(vosk_gpu_init);
DEF_FUNC_PTR(vosk_gpu_thread_init);
#undef EXTERN_FUNC

#define SYM_FUNC(handle, func) func = os_dlsym(handle, #func)

int vosk_wrapper_init() {
    void * handle = os_dlopen("libvosk.dll");
    SYM_FUNC(handle, vosk_model_new);
    SYM_FUNC(handle, vosk_model_free);
    SYM_FUNC(handle, vosk_model_find_word);
    SYM_FUNC(handle, vosk_spk_model_new);
    SYM_FUNC(handle, vosk_spk_model_free);
    SYM_FUNC(handle, vosk_recognizer_new);
    SYM_FUNC(handle, vosk_recognizer_new_spk);
    SYM_FUNC(handle, vosk_recognizer_new_grm);
    SYM_FUNC(handle, vosk_recognizer_set_spk_model);
    SYM_FUNC(handle, vosk_recognizer_set_max_alternatives);
    SYM_FUNC(handle, vosk_recognizer_set_words);
    SYM_FUNC(handle, vosk_recognizer_accept_waveform);
    SYM_FUNC(handle, vosk_recognizer_accept_waveform_s);
    SYM_FUNC(handle, vosk_recognizer_accept_waveform_f);
    SYM_FUNC(handle, vosk_recognizer_result);
    SYM_FUNC(handle, vosk_recognizer_partial_result);
    SYM_FUNC(handle, vosk_recognizer_final_result);
    SYM_FUNC(handle, vosk_recognizer_reset);
    SYM_FUNC(handle, vosk_recognizer_free);
    SYM_FUNC(handle, vosk_set_log_level);
    SYM_FUNC(handle, vosk_gpu_init);
    SYM_FUNC(handle, vosk_gpu_thread_init);
    return 0;
}
