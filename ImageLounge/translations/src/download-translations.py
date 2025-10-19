#!/usr/bin/env python3

import os
import json
import pprint
import requests
import sys

from types import SimpleNamespace
from crowdin_api import CrowdinClient

def dict_to_obj(obj):
    if isinstance(obj, dict):
        # Convert dict into a SimpleNamespace, recurse on values
        return SimpleNamespace(**{k: dict_to_obj(v) for k, v in obj.items()})
    elif isinstance(obj, list):
        # Convert lists recursively
        return [dict_to_obj(item) for item in obj]
    else:
        # Leave other types (str, int, float, None, etc.) as-is
        return obj

apiToken = os.environ["CROWDIN_TOKEN"]
outputDir = sys.argv[1]
projectId = 838858 # nomacs neo

# map Crowdin target language ids that don't match nomacs ids
langMap = {
  "es_ES": "es",
  "pt_PT": "pt",
  "sv_SE": "sv",
}

# use the lower-case version of any of the constants above,
# at least provide token
client = CrowdinClient(token=apiToken)

# find project id
projects = client.projects.list_projects()

projects = dict_to_obj(projects)
#pprint.pprint(projects)
#exit(0);

project = None
for p in projects.data:
    if p.data.id == projectId:
      project = p.data
      break

if project is None:
    raise(RuntimeError("Could not find nomacs neo in projects list"))

# project = projects.data[1].data;
#pprint.pprint(project)

print("Project ID:   %d" % project.id)
print("Project Name: %s" % project.name)
print("Languages:    %s" % project.targetLanguageIds)
# exit(0)

files = client.source_files.list_files(projectId=project.id)
files = dict_to_obj(files)
files = files.data

for f in files:
    f = f.data;
    for t in project.targetLanguageIds:
        print(t, f.id, f.path)

        response = client.translations.build_project_file_translation(
            projectId = project.id,
            fileId = f.id,
            targetLanguageId = t,
            skipUntranslatedStrings = True, # untranslated exported with empty values
        )

        # The build call should return a DownloadFile object (or a URL or some link to fetch)
        url = response.get("data", {}).get("url")
        if not url:
            raise RuntimeError("Could not get download URL from Crowdin response")

        print(url)

        # Download the file
        r = requests.get(url)
        r.raise_for_status()

        sourceName = f.path.replace(".ts","")
        fileId = t.replace("-","_")
        fileId = langMap.get(fileId, fileId)

        outputFile = outputDir + sourceName + "_" + fileId + ".ts"
        os.makedirs(outputDir, exist_ok=True)
        with open(outputFile, "wb") as fh:
            fh.write(r.content)

        print(f"Downloaded translation {f.path}:{t} to {outputFile}")

