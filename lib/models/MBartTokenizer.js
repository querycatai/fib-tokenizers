const utils = require("./utils");

module.exports = function (tokenizers) {
    class MBartTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_eos_token": true,
                "offset": 1,
                "added_tokens_decoder": {
                    "0": "<s>",
                    "1": "<pad>",
                    "2": "</s>",
                    "3": "<unk>",
                },
                "bos_token": "<s>",
                "eos_token": "</s>",
                "unk_token": "<unk>",
                "mask_token": "<mask>",
                "additional_special_tokens": [
                    "ar_AR",
                    "cs_CZ",
                    "de_DE",
                    "en_XX",
                    "es_XX",
                    "et_EE",
                    "fi_FI",
                    "fr_XX",
                    "gu_IN",
                    "hi_IN",
                    "it_IT",
                    "ja_XX",
                    "kk_KZ",
                    "ko_KR",
                    "lt_LT",
                    "lv_LV",
                    "my_MM",
                    "ne_NP",
                    "nl_XX",
                    "ro_RO",
                    "ru_RU",
                    "si_LK",
                    "tr_TR",
                    "vi_VN",
                    "zh_CN"
                ],
                "suffix_tokens": [
                    "en_XX"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.MBartTokenizer = MBartTokenizer;
}