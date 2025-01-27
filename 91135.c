struct mixer_card *uc_get_mixer_for_card(struct audio_usecase *usecase, int card)
{
 struct mixer_card *mixer_card;
 struct listnode *node;

    list_for_each(node, &usecase->mixer_list) {
        mixer_card = node_to_item(node, struct mixer_card, uc_list_node[usecase->id]);
 if (mixer_card->card == card)
 return mixer_card;
 }
 return NULL;
}
