#pragma once

#include "Tokenizer.h"

class FastTokenizer : public Napi::ObjectWrap<FastTokenizer>,
                      public Tokenizer {
public:
    FastTokenizer(const Napi::CallbackInfo& info);
    DECLARE_CLASS(FastTokenizer);
};
