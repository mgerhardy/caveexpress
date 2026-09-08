# Google Play releases

## Release to the internal test track

Do this before tagging a public/production release.

```sh
gh workflow run "Play Store" --ref master \
  -f track=internal \
  -f status=completed \
  -f version=2.7
```

Or GitHub -> **Actions -> Play Store -> Run workflow**: track `internal`, status `completed`, version `2.7`.

That builds signed arm64-v8a AABs, sets `versionName` from the version input, sets `versionCode` to `major * 10000 + minor * 100 + patch` (`2.7` → `20700`, `2.7.1` → `20701`), and uploads to **Internal testing**. Watch with `gh run watch`.

When the job is green: Play Console -> Internal testing -> **Testers** -> copy the opt-in link. Each tester must open that link once; the build then appears in the Play Store app, often after a few minutes.

Closed testing is `alpha`. Open testing is `beta`. Pass those as `track` in the same **Run workflow** dialog.

## Releases from a git tag

Pushing a version tag such as `2.7` or `2.7.1` runs the same workflow. Hyphenated tags (`2.5-dev`) are skipped.

```sh
git tag 2.7
git push origin 2.7
```

`versionName` comes from the tag. `versionCode` uses the formula above. The track is still **internal** unless `PLAY_TRACK` is set to `production` (optionally `PLAY_RELEASE_STATUS=draft`).

## Local AAB build

```sh
export ANDROID_KEYSTORE_PATH=/path/to/upload.keystore
export ANDROID_KEYSTORE_PASSWORD=...
export ANDROID_KEY_ALIAS=caveproductions
export ANDROID_KEY_PASSWORD=...   # optional if same as keystore password
make android-aab
```

Bundles land under `android-project/app/build/outputs/bundle/`.
