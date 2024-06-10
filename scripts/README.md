# Commands for testing

```
parallel --verbose --link -j 16 timeout -s SIGKILL 30m ./exact ::: $(find instances/exact/ -type f | sort -V) ::: $(find instances/exact -type f | sort -V | xargs -I % sh -c 'echo solutions/exact/$(basename % .gr).sol')
```

```
parallel --verbose --link -j 16 timeout 5m ./heuristic ::: $(find instances/heuristic/ -type f | sort -V) ::: $(find instances/heuristic -type f | sort -V | xargs -I % sh -c 'echo solutions/heuristic/$(basename % .gr).sol')
```

```
find instances/exact/ -type f | sort -V | xargs -I % sh -c 'if [ -f solutions/exact/$(basename % .gr).sol ]; then pace2024verifier -c % solutions/exact/$(basename % .gr).sol; else echo "TLE"; fi'
```

```
find instances/heuristic/ -type f | sort -V | xargs -I % sh -c 'pace2024verifier -c % solutions/heuristic/$(basename % .gr).sol'
```