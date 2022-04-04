#include<stdint.h>
#include<stddef.h>
#include<stivale2.h>
#include"sys.h"
#include"GDT.h"
#include"interrupts/IDT.h"
#include"memory/PMM.h"
#include"panic.h"
#include"portio.h"

static uint8_t stack[4096];

static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag =
{
  .tag =
  {
    .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
    .next = 0
  },

  .framebuffer_width  = 0,
  .framebuffer_height = 0,
  .framebuffer_bpp  = 0
};

__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr =
{
  .entry_point = 0,
  .stack = (uintptr_t)stack + sizeof(stack),
  .flags = 0,
  .tags = (uintptr_t)&framebuffer_hdr_tag
};

void* stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id)
{
  struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
  while(1)
  {
    if(current_tag == NULL)
    {
      return NULL;
    }

    if(current_tag->identifier == id)
    {
      return current_tag;
    }

    current_tag = (void *)current_tag->next;
  }
}

struct stivale2_struct_tag_framebuffer* fb_tag;
volatile uint32_t* fb;
extern void kmain(void);

Color fg_col, bg_col;

void _start(struct stivale2_struct *stivale2_struct)
{
  fb_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

  // To display messages
  fg_col = UINT_RGB(0xFFFFFF);
  bg_col = UINT_RGB(0x000000);

	serial_init();

  GDT_init();
  IDT_init();
  
  volatile struct stivale2_struct_tag_memmap* memory_map = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    
  PMM_init(memory_map);
  assert(PMM_alloc0(1), "Error while initializing PMM");

  if(fb_tag == NULL)
  {
    while(1) __asm__ volatile("hlt");
  }

  fb = (uint32_t*)fb_tag->framebuffer_addr;

	current_col = 0;
  current_row = 0;
	clearterm();
  kmain();

  while(1) __asm__ volatile("hlt");
}
