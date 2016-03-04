# drush-create

Provides tools for creating arbitrary nodes, entities, terms and vocabularies with Drush.

# Installation

Add the file to your site's module directory.

# Useage

Create a vocabulary:

    drush ctv mynewvocab my_new_vocab --description="My new vocabulary" --verbose=true

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

# TODO

* Provide a mechanism to create content types and other entities. For fields, use drush-field-create.

* Provide a way to see possible answers in interactive mode

* Provide a way to add entity- and field-references, and field content when creating nodes in batch mode.

* Add fields to vocabularies and field content to terms?

I am looking for people to help maintain this project. Please submit an issue to get involved.
