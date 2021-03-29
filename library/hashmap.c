struct hlist_node {
	struct hlist_node *next, **pprev;
};

struct hlist_head {
	struct hlist_node *first;
};

#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

static inline void __hash_init(struct hlist_head *ht, unsigned int sz)
{
	unsigned int i;

	for (i = 0; i < sz; i++)
		INIT_HLIST_HEAD(&ht[i]);
}

#define MAP_HASH_SIZE(bits) (1 << bits)
#define hash_init(map) \
	__hash_init(map->ht, MAP_HASH_SIZE(map->bits))

typedef struct {
	int bits;
	struct hlist_head *ht;
} Map;

Map* map_init(int bits)
{
	Map *map;

	map = (Map *)malloc(sizeof(Map));
	if (map) {
		map->bits = bits;
		map->ht = (struct hlist_head *)malloc(sizeof(struct hlist_head) * MAP_HASH_SIZE(map->bits));
		if (map->ht) {
			hash_init(map);
		} else {
			free(map);
			map = NULL;
		}
	}

	return map;
}

/* map data */
struct hash_key {
	int key;
	void *data;
	struct hlist_node node;
};


/* map find */

#define container_of(ptr, type, member) ({		\
	void *__mptr = (void *)(ptr);			\
	((type *)(__mptr - offsetof(type, member))); })

#define GOLDEN_RATIO_32	0x61C88647
static inline unsigned int hash_32(unsigned int val, unsigned int bits)
{
	/* High bits are more random, so use them. */
	return (val * GOLDEN_RATIO_32) >> (32 - bits);
}

struct hash_key *find_key(Map *map, int key)
{
	struct hash_key *kn;
	struct hlist_head *head;
	struct hlist_node *p;

	head = &((map->ht)[hash_32(key, map->bits)]);
	p = head->first;

	while (p) {
		kn = container_of(p, struct hash_key, node);
		if (kn->key == key)
			return kn;
		p = p->next;
	}
	return NULL;
}


/* map get */
void *map_get(Map *map, int key)
{
	struct hash_key *kn;

	kn = find_key(map, key);
	return (kn) ? kn->data : NULL;
}

/* map add */
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;

	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

#define hash_add(ht, node, key, bits) \
	hlist_add_head(node, &ht[hash_32(key, bits)])

void map_add(Map *map, int key, void *data)
{
	struct hash_key *kn;

	kn = find_key(map, key);
	if (!kn) {
		kn = (struct hash_key *)malloc(sizeof(struct hash_key));
		kn->key = key;
		kn->data = data;
		hash_add(map->ht, &kn->node, key, map->bits);
	}
}

/* map del */
static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;

	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline void hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hash_del(struct hlist_node *node)
{
	hlist_del_init(node);
}

void *map_remove(Map *map, int key)
{
	struct hash_key *kn;
	void *data = NULL;

	kn = find_key(map, key);
	if (kn) {
		data = kn->data;
		hash_del(&kn->node);
		free(kn);
	}
	return data;
}

/* map deinit */
void map_deinit(Map *map)
{
	struct hlist_head *head;
	struct hlist_node *p;

	if (!map)
		return;

	for (int i = 0; i < MAP_HASH_SIZE(map->bits); i++) {
		head = &(map->ht[i]);
		p = head->first;

		while (p) {
			struct hlist_node *prev;
			struct hash_key *kn;

			kn = container_of(p, struct hash_key, node);
			prev = p;
			p = p->next;
			hash_del(prev);
			if (kn->data)
				free(kn->data);
			free(kn);
		}
	}

	free(map);
}

