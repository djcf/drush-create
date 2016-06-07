# drush-create

Provides tools for creating arbitrary nodes, entities, terms and vocabularies with Drush.

# Installation

Install as command extension in your local drush configuration

    cd ~/.drush
    git clone <this repository>

# Installation (other)

**Warning:** These instructions are outdated and do no longer work!

Add the file to your site's module directory. For example:

    mkdir -p sites/all/drush
    curl "https://raw.githubusercontent.com/djcf/drush-create/master/create-content.drush.inc" > sites/all/drush/create-content.drush.inc

Or put it where drush will be able to find it for all sites:

    mkdir -p /usr/share/drush/commands
    curl "https://raw.githubusercontent.com/djcf/drush-create/master/create-content.drush.inc" > /usr/share/drush/commands/create-content.drush.inc
    

## Additional requirements

The `create-content` command requires the drupal enitity module to be installed. You can easily install it by drush:

    drush dl entity
    drush en entity

# Usage

Create a vocabulary:

    drush ctv mynewvocab my_new_vocab --description="My new vocabulary" --verbose=true

If you have drush_taxonomyinfo (https://www.drupal.org/project/drush_taxonomyinfo), you can verify that the new vocabulary was created successfully:

	drush tvl

This function prints the new vID. Use it to create a term for your new vocab:

    drush ctt newterm 1 --parent_id=10 --description="My new term" --verbose=true

This function prints the new tID. Now create a node:

    drush cnn article \
	--title="foo"
	--author=1
	--language=LANGUAGE_NONE \
	--comments=2 \
	--status=1 \
	--sticky=0 \
	--promote=0 \
	--body="Hello, world" \
	--input_format="filtered_html", \
	--verbose

Adding custom fields in batch mode is a little trickier.

Using the --vocabularies option, specify a colon-delimitted map of the field machine name and the vocabulary machine name, separated by commas like so:

    drush cnn article --vocabularies=field_x:vocab_x,field_y:vocab_y,field_z:vocab_z ...

You can now add the taxonomy terms using either their existing tid or their name. Terms will be created automatically if they do not exist already:

    drush cnn article --vocabularies=field_x:vocab_x,field_y:vocab_y,field_z:vocab_z \
	--field_x:a,b,c
	--field_y:x,y,z
	--field_z:foo,bar,baz
	--strict=0

You **must** append "--strict=0" to prevent drush from complaining about your arbitrary --field_x options.

You can add entity references in a similar way, however the options must be node IDs:

    drush cnn article --fields=field_x,field_y,field_z \
	--field_x:1,2,3
	--field_y:4,5,6
	--field_z:7,8,9
	--strict=0

If the node with --nid=x already exists, the existing node will be automatically updated with the new content.

For all commands you can specify --auto, which means that only the new ID will be printed. If the term or vocabulary already exists, no error is produced, but the old ID is still output. This is excellent for running as a sub-process for some other operation.

# TODO

* Provide a mechanism to create content types and other entities. For fields, use drush-field-create.

* Provide a way to see possible answers in interactive mode

* Add field content to terms?

I am looking for people to help maintain this project. Please submit an issue to get involved.
