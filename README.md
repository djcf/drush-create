# drush-create

Provides tools for creating arbitrary nodes, entities, terms and vocabularies with Drush.

# Installation

# Useage

Create a vocabulary:

    drush ctv mynewvocab my_new_vocab --description="My new vocabulary" --verbose=true

This function prints the new vID. Use it to create a term for your new vocab:

    drush ctt newterm 1 --parent_id=10 --description="My new term" --verbose=true

This function prints the new tID. Now create a node:



# TODO

Provide a mechanism for content types and other entities. For fields, use drush-field-create.
