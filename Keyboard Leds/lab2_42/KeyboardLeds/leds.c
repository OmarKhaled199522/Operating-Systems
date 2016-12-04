																																																																					#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/semaphore.h>

static struct semaphore sema;
static int caps;
static int num;
static int scroll;
unsigned char all_leds_state = 0;

unsigned char kbd_read_status(void){
    return inb(0x64);
}

unsigned char kbd_read_data(void){
    
    unsigned char register_status; 
    while( !( (register_status = kbd_read_status()) & 1) );
    unsigned char data = inb(0x60);
    return data;
}

void kbd_write_data(unsigned char data){

    unsigned char register_status; 
    while( (register_status = kbd_read_status()) & 2);
    outb(data, 0x60);	
}

int update_leds(unsigned char led_status_word) {

    
    printk("id/pid %d\n: starting", current->pid);
    disable_irq(1);

    down(&sema); // enter critical region
    // send ’Set LEDs’ command
    printk("id/pid %d\n: sending oxED command", current->pid);
    kbd_write_data(0xED);
    printk("id/pid %d\n: waiting for ack", current->pid);
    // wait for ACK
    if (kbd_read_data() != 0xFA)
        return -1;

    printk("id/pid %d\n: received ack", current->pid);
    printk("id/pid %d\n: sleeping for 500ms", current->pid);
    msleep(500);
    // now send LED states
    printk("id/pid %d\n: wake up", current->pid);
    printk("id/pid %d\n: sending keyboard data", current->pid);
    kbd_write_data(led_status_word);
    printk("id/pid %d\n: waiting for another ack", current->pid);
    // wait for ACK
    if (kbd_read_data() != 0xFA)
	return -1;

    printk("id/pid %d\n: received another ack", current->pid);
    printk("id/pid %d\n: exit", current->pid);
    // success

    up(&sema); // exit critical region
    enable_irq(1);
    return 0;
}

void set_led_state(int led, int state){

    all_leds_state &= ~(1 << led);
    all_leds_state |= (state << led);
    update_leds(all_leds_state);
}

int get_led_state(int led){

    down(&sema); // enter critical region
    return ( (1 << led) & all_leds_state);
    up(&sema); // exit critical region
}

/*
 * The "caps" file where a static variable is read from and written to.
 */
static ssize_t caps_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", caps);
}

static ssize_t caps_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &caps);
	if (ret < 0)
		return ret;

	set_led_state(2, caps);
	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute caps_attribute =
	__ATTR(caps, 0664, caps_show, caps_store);

/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "num" and "scroll" files.
 */
static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "num") == 0)
		var = num;
	else
		var = scroll;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;

	if (strcmp(attr->attr.name, "num") == 0){
		num = var;
		set_led_state(1, num);
	} else {
		scroll = var;
		set_led_state(0, scroll);
	}
	return count;
}

static struct kobj_attribute num_attribute =
	__ATTR(num, 0664, b_show, b_store);
static struct kobj_attribute scroll_attribute =
	__ATTR(scroll, 0664, b_show, b_store);


/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&caps_attribute.attr,
	&num_attribute.attr,
	&scroll_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj;

int init_module(void)
{
    	//Called when installed
    	//printk(KERN_INFO "Hello world!\n");
	sema_init(&sema, 1);
	int retval;

	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	example_kobj = kobject_create_and_add("keyboard", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(example_kobj, &attr_group);
	if (retval)
		kobject_put(example_kobj);

    	//update_leds(7);
	return retval;
}

void cleanup_module(void)
{
    //Called when removed
    printk(KERN_INFO "Goodbye world!\n");
}

MODULE_LICENSE("GPL v2");
