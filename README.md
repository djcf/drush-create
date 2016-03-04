# drush-create

Provides tools for creating arbitrary nodes, entities, terms and vocabularies with Drush.

# Installation

Add the file to your site's module directory. For example:

    mkdir -p sites/all/drush
    curl "https://raw.githubusercontent.com/djcf/drush-create/master/create-content.drush.inc" > sites/all/drush/create-content.drush.inc

Or put it where drush will be able to find it for all sites:

    mkdir -p /usr/share/drush/commands
    curl "https://raw.githubusercontent.com/djcf/drush-create/master/create-content.drush.inc" > /usr/share/drush/commands/create-content.drush.inc

# Useage

Create a vocabulary:

    drush ctv mynewvocab my_new_vocab --description="My new vocabulary" --verbose=true

If you have drush_taxonomyinfo (https://www.drupal.org/project/drush_taxonomyinfo), you can verify that the new vocabulary was created successfully:

	drush tvl

This function prints the new vID. Use it to create a term for your new vocab:

    drush ctt newterm 1 --parent_id=10 --description="My new term" --verbose=true

This function prints the new tID. Now create a node:

    drush cnn newnode 100 \
	--language=LANGUAGE_NONE \
	--comments=2 \
	--status=1 \
	--sticky=0 \
	--promote=0 \
	--body="Hello, world" \
	--input_format="filtered_html", \
	--verbose

You can also run each command interactively, which is currently the only way to add entity- and field-references.

For all commands you can specify --auto, which means that only the new ID will be printed. This is excellent for running as a sub-process for some other operation.

# TODO

* Provide a mechanism to create content types and other entities. For fields, use drush-field-create.

* Provide a way to see possible answers in interactive mode

* Provide a way to add entity- and field-references, and field content when creating nodes in batch mode.

* Add fields to vocabularies and field content to terms?

I am looking for people to help maintain this project. Please submit an issue to get involved.
