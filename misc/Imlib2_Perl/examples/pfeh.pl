#!/usr/bin/perl
use Imlib2;
use strict;
eval 'use Gtk; init Gtk;' or die "Did not find Gtk::Perl\n";

init Gtk::Gdk::Rgb;
Gtk::Widget->set_default_colormap(Gtk::Gdk::Rgb->get_cmap());
Gtk::Widget->set_default_visual(Gtk::Gdk::Rgb->get_visual());
my $inited = 0;

parse_args();
load_image();
Gtk->main();

sub imlib_init
{
	my ($win) = @_;
	return 1 if $inited;
	my $cmap = $win->{'da'}->get_colormap()->XCOLORMAP;
	my $visual = $win->{'da'}->get_visual()->XVISUAL;
	my $display = $win->{'da'}->window->XDISPLAY;
	Imlib2::context_set_display($display);
	Imlib2::context_set_colormap($cmap);
	Imlib2::context_set_visual($visual);
	$inited = 1;          
	return 1;
}

sub load_image
{
	my $image = Imlib2::load_image($ARGV[0]);
	Imlib2::set_context($image);
	print("image loaded\n");
	my $w = Imlib2::get_width();
	my $h = Imlib2::get_height();
	show_image($image,$w,$h);
	return 1;
}	

sub show_image
{
	my ($image,$w,$h) = @_;
	my $win;
	$win = new Gtk::Window("toplevel");
	$win->set_title("Viewing " . $ARGV[0]);
	$win->set_default_size($w,$h);
	$win->signal_connect("destroy", sub { Gtk->exit(0) });
	$win->{'da'} = new Gtk::DrawingArea();
	$win->{'da'}->size($w,$h);
	$win->add($win->{'da'});
	$win->{'da'}->realize();
	imlib_init($win);
	
	my $bgcolor = Gtk::Gdk::Color->parse_color('white');
	$bgcolor = $win->{'da'}->window->get_colormap()->color_alloc($bgcolor);
	$win->{'da'}->window->set_background($bgcolor);
	

	Imlib2::context_set_drawable($win->{'da'}->window->XWINDOW);
	$win->{'da'}->signal_connect('expose_event', \&render);	


	show_all $win;
	return 1;
}	

sub render
{
	print("expose_event\n");
	Imlib2::render_image_on_drawable(0,0);
	Imlib2::free_image();

}	

sub parse_args
{
	if (!$ARGV[0]) {
		die "usage: pfeh.pl image\n";
	} else {
		if ($ARGV[0] !~ /(bmp|gif|jpg|jpeg|bmp|png)$/i) {
			die "$ARGV[0] is not an image\n";
		}
	}
	
	return 1;
}	
