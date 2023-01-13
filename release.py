import os
import re
import sys


def exec(cmd):
    print(">>>", cmd)
    os.system(cmd)


def replace(file, what, to):
    with open(file, "r+") as f:
        content = f.read()
        content = re.sub(what, to, content)

        f.seek(0)
        f.truncate(0)
        f.write(content)

    exec(f"git add {file}")


if __name__ == "__main__":
    version = sys.argv[1]

    exec("git checkout master")

    replace("build_ext.py", r'VERSION\s*=\s*"(.*?)"', f'VERSION = "{version}"')
    replace("pyproject.toml", r'version\s*=\s*"(.*?)"', f'version = "{version}"')

    exec(f'git commit -m "chore(bump): {version}"')
    exec("git checkout release")
    exec("git merge master")
    exec(f'git tag -a {version} -m "chore(bump): {version}"')
    exec("git push && git push --tags")
    exec("git checkout master")
    exec("git merge release")
