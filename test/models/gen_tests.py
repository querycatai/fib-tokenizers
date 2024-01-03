import requests
import json
import os
import re
import glob
import shutil
from transformers import AutoTokenizer
from colorama import Fore

TEST_DATA = {
    "shared": [
        "hello world",
        "Hello World",
        "How are you doing?",
        "You should've done this",
        "A\n'll !!to?'d''d of, can't.",
        "def main():\n\tpass",
        "This\n\nis\na\ntest.",
        "let a = obj.toString();\ntoString();",
        'Hi  Hello',
        "trailing space   ",
        "   leading space",
        "生活的真谛是",
        "The company was founded in 2016.",
        "test $1 R2 #3 €4 £5 ¥6 ₣7 ₹8 ₱9 test",
        "I bought an apple for $1.00 at the store.",
        "you…  ",
        "\u0079\u006F\u0075\u2026\u00A0\u00A0",
        "\u0079\u006F\u0075\u2026\u00A0\u00A0\u0079\u006F\u0075\u2026\u00A0\u00A0",
        "▁This ▁is ▁a ▁test ▁.",
        "weird \uFF5E edge \uFF5E case",

        # SentencePiece-specific test cases
        "<s>\n",
        " </s> test </s> ",
        "</s>test</s>",
        # Additional test-cases for the Llama tokenizer, adapted from
        # https://github.com/belladoreai/llama-tokenizer-js/blob/master/llama-tokenizer.js#L381-L452
        "grabbed",
        " grabbed",
        "           grabbed",
        "\n",
        " \n",
        "	tabs				out here",
        "\n\t\n",
        "ax\n####\nboo",
        "镇",
        "🦙",
        "🦙Ꙋ",
        "Ꙋ🦙",
        "The llama (/ˈlɑːmə/; 🦙Spanish pronunciation: [ˈʎama]) (Lama glama) is a domesticated South American " \
        "camelid, widely used as a meat and pack animal by Andean cultures since the Pre-Columbian era. Llamas " \
        "are social animals and live with others as a herd. Their wool is soft and contains only a small " \
        "amount of lanolin.[2] Llamas can learn simple tasks after a few repetitions. When using a pack, they " \
        "can carry about 25 to 30% of their body weight for 8 to 13 km (5–8 miles).[3] The name llama (in the " \
        "past also spelled \"lama\" or \"glama\") was adopted by European settlers from native Peruvians.[4] " \
        "The ancestors of llamas are thought to have originated from the Great Plains of North America about " \
        "40 million years ago, and subsequently migrated to South America about three million years ago during " \
        "the Great American Interchange. By the end of the last ice age (10,000–12,000 years ago), camelids were " \
        "extinct in North America.[3] As of 2007, there were over seven million llamas and alpacas in South " \
        "America and over 158,000 llamas and 100,000Ꙋ🦙 alpacas, descended from progenitors imported late in " \
        "the 20th century, in the United States and Canada.[5] In Aymara mythology, llamas are important beings. " \
        "The Heavenly Llama is said to drink water from the ocean and urinates as it rains.[6] According to " \
        "Aymara eschatology, llamas will return to the water springs and lagoons where they come from at the " \
        "end of time.[6]",
    ]
}

def resolve_model(model):
    home = os.path.join(os.environ['HOME'], ".cache/huggingface/hub")
    model_home = os.path.join(home, "models--" + model.replace('/', '--'))
    if os.path.exists(os.path.join(model_home, "refs")) and os.path.exists(os.path.join(model_home, "snapshots")):
        with open(os.path.join(model_home, "refs", "main"), 'r') as file:
            tag = file.read().replace('\n', '')
        return os.path.join(model_home, "snapshots", tag)
    else:
        return model_home

def generate_one(model, likes):
    model_file = "models--" + model.replace("/", "--") + ".json"
    if os.path.exists(model_file):
        return

    try:
        print(f"{Fore.GREEN}Generating for {model}{Fore.RESET}")
        tokenizer = AutoTokenizer.from_pretrained(model, trust_remote_code=True, use_fast=False)

        model_home = resolve_model(model)
        tokenizer_json = os.path.join(model_home, "tokenizer.json")
        if os.path.isfile(tokenizer_json) and glob.glob(os.path.join(model_home, "*.model")):
            os.remove(tokenizer_json)

        tokenizer = AutoTokenizer.from_pretrained(model, trust_remote_code=True, use_fast=False, local_files_only=True)

        tokenizer_class = tokenizer.__class__.__name__.replace("Fast", "")
        special_tokens=tokenizer.all_special_tokens

        datasets = []
        shared_texts = TEST_DATA["shared"].copy()
        shared_texts += ["<s></s><unk><pad>" + ''.join(special_tokens) ]

        for text in shared_texts:
            ids = tokenizer.encode(text)

            tokens = tokenizer.tokenize(text)
            if tokens and isinstance(tokens[0], bytes):
                tokens = [token.decode('utf-8', errors='ignore') for token in tokens]

            if tokenizer_class == "ChatGLMTokenizer":
                decoded = tokenizer.decode(ids[2:], skip_special_tokens=True)
            else:
                decoded = tokenizer.decode(ids, skip_special_tokens=True)

            # decoded_with_special = tokenizer.decode(ids, skip_special_tokens=False)

            datasets.append(dict(
                input=text,
                ids=ids,
                tokens=tokens,
                decoded=decoded,
                # decoded_with_special=decoded_with_special
            ))

        with open(model_file, "w", encoding="utf-8", errors='ignore') as fp:
            json.dump(dict(
                model=model,
                tokenizer_class=tokenizer_class,
                likes=likes,
                special_tokens=special_tokens,
                datasets=datasets,
            ), fp, indent=4)
    except Exception as e:
        cache_dir = os.path.join(os.path.expanduser("~"), ".cache", "huggingface", "hub", "models--" + model.replace("/", "--"))
        shutil.rmtree(cache_dir, ignore_errors=True)
        print(f"\n{Fore.RED}===========> Failed to generate for {model}: {e}{Fore.RESET}\n")

def renew_model(model):
    model = re.search(r'models--(.*).json', model)[1].replace('--', '/')
    try:
        print(f"{Fore.GREEN}Renew for {model}{Fore.RESET}")
        tokenizer = AutoTokenizer.from_pretrained(model, trust_remote_code=True, use_fast=False)
    except Exception as e:
        cache_dir = os.path.join(os.path.expanduser("~"), ".cache", "huggingface", "hub", "models--" + model.replace("/", "--"))
        shutil.rmtree(cache_dir, ignore_errors=True)
        print(f"\n{Fore.RED}===========> Failed to generate for {model}: {e}{Fore.RESET}\n")

local_models = glob.glob(os.path.join("./", "models--*.json"))
for model in local_models:
    renew_model(model)

response = requests.get("https://huggingface.co/api/models", params={
    'sort': 'likes',
    'direction': -1,
    'limit': 2000
})

models = [model['modelId'] for model in response.json()]

for model in response.json():
    generate_one(model['modelId'], model['likes'])
