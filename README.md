# PACE2024-UiB

```
parallel --verbose --link -j 6 timeout 10m ./exact ::: $(find instances/exact/ -type f | sort -V) ::: $(find instances/exact -type f | sort -V | xargs -I % sh -c 'echo solutions/exact/$(basename % .gr).sol')
```