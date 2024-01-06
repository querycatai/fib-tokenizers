const utils = require("./utils");

module.exports = function (tokenizers) {
    class MBart50Tokenizer extends tokenizers.SentencepieceTokenizer {
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
                    "zh_CN",
                    "af_ZA",
                    "az_AZ",
                    "bn_IN",
                    "fa_IR",
                    "he_IL",
                    "hr_HR",
                    "id_ID",
                    "ka_GE",
                    "km_KH",
                    "mk_MK",
                    "ml_IN",
                    "mn_MN",
                    "mr_IN",
                    "pl_PL",
                    "ps_AF",
                    "pt_XX",
                    "sv_SE",
                    "sw_KE",
                    "ta_IN",
                    "te_IN",
                    "th_TH",
                    "tl_XX",
                    "uk_UA",
                    "ur_PK",
                    "xh_ZA",
                    "gl_ES",
                    "sl_SI"
                ],
                "prefix_tokens": [
                    "en_XX"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.MBart50Tokenizer = MBart50Tokenizer;
}