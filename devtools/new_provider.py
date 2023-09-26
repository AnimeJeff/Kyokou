import os

try:
    provider_name = input("Enter Provider Name: ")
    outputPath = os.path.join(os.path.dirname(__file__),"..\src\Providers",provider_name.lower())
    with open(os.path.join(os.path.dirname(__file__),"template.txt"),"r") as f:
        template = f.read()
        template = template.replace("NewProvider",provider_name).replace("NEWPROVIDER",provider_name.upper())
    with open(f"{outputPath}.h","w") as f:
        f.write(template)
    with open(f"{outputPath}.cpp","w") as f:
        f.write(f"#include \"{provider_name.lower()}.h\"")
except KeyboardInterrupt:
    if provider_name:
        if os.path.exists(f"{outputPath}.h"):
            os.remove(f"{outputPath}.h")
        if os.path.exists(f"{outputPath}.cpp"):
            os.remove(f"{outputPath}.cpp")