const class_list = [
    // SentencepieceTokenizer
    "LlamaTokenizer",
    "T5Tokenizer",
    "ChatGLMTokenizer",
    "CodeLlamaTokenizer",
    "DebertaV2Tokenizer",
    "BaichuanTokenizer",
    "XLMRobertaTokenizer",
    "InternLMTokenizer",
    "NllbTokenizer",
    "PegasusTokenizer",
    "MBart50Tokenizer",
    "YiTokenizer",
    "CamembertTokenizer",
    "SpeechT5Tokenizer",
    "SeamlessM4TTokenizer",
    "PlamoTokenizer",
    "MBartTokenizer",
    "SkyworkTokenizer",
    "XGLMTokenizer",
    "XLNetTokenizer",
    "Midm_bitext_Tokenizer",
    "BigBirdTokenizer",

    // BpeTokenizer
    "GPT2Tokenizer",
    "CLIPTokenizer",
    "CodeGenTokenizer",
    "WhisperTokenizer",
    "RobertaTokenizer",
    "BartTokenizer",
    "BlenderbotTokenizer",
    "OpenAIGPTTokenizer",
    "LongformerTokenizer",
    "FLMTokenizer",
    "LEDTokenizer",

    // BertTokenizer
    "BertTokenizer",
    "DistilBertTokenizer",
    "MPNetTokenizer",
    "ElectraTokenizer",

    // TikTokenizer
    "QWenTokenizer"
];

module.exports = function (tokenizers) {
    class_list.forEach(model => require(`./${model}`)(tokenizers));
}