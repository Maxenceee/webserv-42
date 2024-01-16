'use strict'

const fs = require('fs')
const path = require('path')

const toJSON = function(conf, options = {}) {
    // split multi-line string to array of lines. Remove TAB characters
    const lines = conf.replace('\t', '').split('\n')
    // const json = {} // holds constructed json
    let parent = '' // parent keys as we descend into object
    let chunkedLine = null // aggregator for multi-lines directives
    let innerLines = [] // array for blocks extracted from multi-blocks line
    let countOfParentsThatAreArrays = 0 // how many of the parent keys are arrays

    lines.forEach(lineRaw => {
      lineRaw = lineRaw.trim() // prep for `startsWith` and `endsWith`

      // If line is blank line or is comment, do not process it
      if (!lineRaw || lineRaw.startsWith('#')) return

      // Line can contain comments, we need to remove them
      lineRaw = lineRaw.split('#')[0].trim()

      /*
        Line can contain multiple blocks
        e.g. "upstream x {server A;} upstream y {server B; server C; server D;}"
        Wrap curly brackets in ' {' and '; }' with new line symbols.
        Add new line after all ';',
      */
      innerLines = lineRaw
        .replace(/(\s+{)/g, '\n$1\n')
        .replace(/(;\s*)}/g, '$1\n}\n')
        .replace(/;\s*?$/g, ';\n')
        .split(/\n/)

      innerLines.forEach(line => {
        line = line.trim()
        if (!line) return

        // If we're in a lua block, append the line to the luaBlockValue and continue

        chunkedLine && (line = chunkedLine + ' ' + line)

        /*
          1. Object opening line
          Append key name to `parent` and create the sub-object in `json`
          e.g. for the line "location /api {", `json` is extended with
          the following key/value:
          { "location /api": {} }
        */
        if (line.endsWith('{')) {
          chunkedLine = null
          const key = line.slice(0, line.length - 1).trim();

          // If we are already a level deep (or more), add a dot before the key
          if (parent) parent += '.' + key
          // otherwise just track the key
          else parent = key

          // store in constructed `json` (support array resolving)
		  console.log(key, "", parent);
        //   if (this.appendValue(json, parent, {})) {
        //     // Array was used and we need to update the parent key with an index
        //     parent += '.' + (this.resolve(json, parent).length - 1)
        //     countOfParentsThatAreArrays += 1
        //   }
        }
        /*
          3. Standard property line
          Create a key/value pair in the constructed `json`, which
          reflects the key/value in the conf file.
        */
        else if (line.endsWith(';')) {
          chunkedLine = null
          line = line.split(' ')

          // Put the property name into `key`
          let key = line.shift();
          // Put the property value into `val`
          let val = line.join(' ').trim()

          // If key ends with a semi-colon, remove that semi-colon
          if (key.endsWith(';')) key = key.slice(0, key.length - 1)
          // Remove trailing semi-colon from `val` (we established its
          // presence already)
          val = val.slice(0, val.length - 1)
       		 //   this.appendValue(json, key, val, parent)
			console.log(key, val, parent);
        }
        /*
          4. Object closing line
          Removes current deepest `key` from `parent`
          e.g. "server.location /api" becomes "server"
        */
        else if (line.endsWith('}')) {
          chunkedLine = null
          // If we're in a lua block, make sure the final value gets stored before moving up a level

          // Pop the parent to go lower
          parent = parent.split('.')

          // check if the current level is an array
          if (countOfParentsThatAreArrays > 0 && !isNaN(parseInt(parent[parent.length - 1], 10))) {
            parent.pop() // remove the numeric index from parent
            countOfParentsThatAreArrays -= 1
          }
          parent.pop()
          parent = parent.join('.')
        }
        /*
          5. Line may not contain '{' ';' '}' symbols at the end
          e.g. "location /api
                { ... }"
          Block begins from the new line here.
        */
        else {
          chunkedLine = line
        }
      })
    })

    // return json
}

let configString = fs.readFileSync(path.resolve(__dirname, '../../config.conf'))

if (Buffer.isBuffer(configString)) {
	configString = configString.toString('utf8')
}

toJSON(configString);