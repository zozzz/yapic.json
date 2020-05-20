# Developemnt

## Release Process

- change ``VERSION`` in ``setup.py``
- ``git add setup.py``
- ``git commit -m "chore(bump): VERSION"``
- ``git checkout release``
- ``git merge master``
- ``git tag -a VERSION -m "chore(bump): VERSION"``
- ``git push && git push --tags``
- ``git checkout master``
- ``git merge release``
