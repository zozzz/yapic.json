import sys
import os
import re


def exec(cmd):
    print(">>>", cmd)
    os.system(cmd)


if __name__ == "__main__":
    version = sys.argv[1]

    exec('git checkout master')

    with open("setup.py", "r+") as f:
        content = f.read()
        content = re.sub(r'VERSION\s*=\s*"(.*?)"', f'VERSION = "{version}"', content)

        f.seek(0)
        f.truncate(0)
        f.write(content)

    exec('git add setup.py')
    exec(f'git commit -m "chore(bump): {version}"')
    exec('git checkout release')
    exec('git merge master')
    exec(f'git tag -a {version} -m "chore(bump): {version}"')
    exec('git push && git push --tags')
    exec('git checkout master')
    exec('git merge release')
